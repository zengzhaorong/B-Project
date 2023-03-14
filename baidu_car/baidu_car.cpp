#include <stdio.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include "capture.h"
#include "json/json.h"
#include "base64.h"
#include "config.h"
#include "baidu_car.h"

using namespace std;

// libcurl库下载链接：https://curl.haxx.se/download.html
// jsoncpp库下载链接：https://github.com/open-source-parsers/jsoncpp/

std::string baidu_access_token;

const static std::string access_token_url = "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials";
const static std::string license_plate_url = "https://aip.baidubce.com/rest/2.0/ocr/v1/license_plate";

license_plate_info_t g_license_info;
bool g_license_valid = 0;

int car_set_license_info(license_plate_info_t *license_info)
{
    memcpy(&g_license_info, license_info, sizeof(license_plate_info_t));
    g_license_valid = 1;

    return 0;
}

int car_get_license_info(license_plate_info_t *license_info)
{

    if(!g_license_valid)
        return -1;

    memcpy(license_info, &g_license_info, sizeof(license_plate_info_t));

    return 0;
}

void car_clear_license_info(void)
{
    g_license_valid = 0;
}

/**
 * curl发送http请求调用的回调函数，回调函数中对返回的json格式的body进行了解析，解析结果储存在result中
 * @param 参数定义见libcurl库文档
 * @return 返回值定义见libcurl库文档
 */
static size_t _access_token_cb(void *ptr, size_t size, size_t nmemb, void *stream)
{
    // 获取到的body存放在ptr中，先将其转换为string格式
    std::string s((char *) ptr, size * nmemb);
    //cout << "recv: " << s << endl;

    Json::Reader reader;
    Json::Value root;
    // 使用boost库解析json
    reader.parse(s,root);
    std::string *access_token_result = static_cast<std::string*>(stream);
    *access_token_result = root["access_token"].asString();
    return size * nmemb;
}

static size_t _license_plate_cb(void * buffer, size_t size, size_t nmemb, void * userp)
{
    std::string * str = dynamic_cast<std::string *>((std::string *)userp);
    str->append((char *)buffer, size * nmemb);
    return size * nmemb;
}

int baidu_parse_license(const std::string detect_result, license_plate_info_t *license_info)
{
    Json::Reader reader;
    Json::Value root;
    Json::Value json_result;
    Json::Value json_number;
    bool bret;

    bret = reader.parse(detect_result, root);
    if(!bret)
    {
        cout << "ERROR: parse json error!" << endl;
        return -1;
    }

    json_result = root["words_result"];
    if(json_result.isNull())
        return -1;

    json_number = json_result["number"];
    if(json_number.isNull())
        return -1;

    cout << "license plate: " << json_number.asString() << endl;
    strcpy(license_info->license, json_number.asString().c_str());

    return 0;
}

/**
 * 用以获取access_token的函数，使用时需要先在百度云控制台申请相应功能的应用，获得对应的API Key和Secret Key
 * @param access_token 获取得到的access token，调用函数时需传入该参数
 * @param AK 应用的API key
 * @param SK 应用的Secret key
 * @return 返回0代表获取access token成功，其他返回值代表获取失败
 */
int baidu_get_access_token(std::string &access_token, const std::string &AK, const std::string &SK)
{
    CURL *curl;
    CURLcode result_code;
    int error_code = 0;
    curl = curl_easy_init();
    if (curl) {
        std::string url = access_token_url + "&client_id=" + AK + "&client_secret=" + SK;
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        std::string access_token_result;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &access_token_result);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _access_token_cb);
        result_code = curl_easy_perform(curl);
        if (result_code != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));
            return 1;
        }
        access_token = access_token_result;
        cout << "access token: " << access_token << endl;

        curl_easy_cleanup(curl);
        error_code = 0;
    } else {
        fprintf(stderr, "curl_easy_init() failed.");
        error_code = 1;
    }
    return error_code;
}

int baidu_license_plate_recogn(license_plate_info_t *license_info, const std::string &access_token, const char *image)
{
    std::string url = license_plate_url + "?access_token=" + access_token;
    std::string image_urlEncode;
    std::string plate_result;
    CURL *curl;
    CURLcode res;
    int ret = -1;

    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        image_urlEncode = "image=";
        image_urlEncode += curl_escape(image, strlen(image));

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, image_urlEncode.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &plate_result);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _license_plate_cb);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
        {
            cout << "curl_easy_perform failed!" << endl;
            goto _EXIT;
        }

        ret = baidu_parse_license(plate_result, license_info);
        if(ret != 0)
        {
            cout << "parse license failed!" << endl;
            goto _EXIT;
        }
    }

_EXIT:
    curl_easy_cleanup(curl);

    return ret;
}

int baidu_car_init(void)
{
    int ret;

    ret = baidu_get_access_token(baidu_access_token, BAIDU_API_KEY, BAIDU_SECRET_KEY);
    if(ret != CURLE_OK)
    {
        printf("%s failed!\n", __FUNCTION__);
        return -1;
    }

    printf("%s ok.\n", __FUNCTION__);

    return 0;
}

unsigned char image_data[FRAME_BUF_SIZE];
unsigned char image_base64[IMAGE_BASE64_SIZE];

void *baidu_car_thread(void *arg)
{
    license_plate_info_t license_info;
    static int old_frame_index = 0;
    int image_len;
    int base64_len;
    int ret;

    (void)arg;

    baidu_car_init();

    while(1)
    {
        memset(image_data, 0, sizeof(image_data));
        ret = capture_get_newframe((unsigned char *)image_data, FRAME_BUF_SIZE, &image_len);
        if(ret <=0 || ret == old_frame_index)
        {
            //printf("%s: no new frame.\n", __FUNCTION__);
            usleep(100 *1000);
            continue;
        }
        old_frame_index = ret;

        memset(image_base64, 0, sizeof(image_base64));
        base64_encode(image_data, image_len, image_base64, &base64_len);

        baidu_license_plate_recogn(&license_info, baidu_access_token, (const char *)image_base64);

        car_set_license_info(&license_info);

        sleep(1);
    }

    return NULL;
}


int start_baidu_car_task(void)
{
    pthread_t tid;
    int ret;

    ret = pthread_create(&tid, NULL, baidu_car_thread, NULL);
    if(ret != 0)
    {
        return -1;
    }

    return 0;
}
