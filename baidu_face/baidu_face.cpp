#include <stdio.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include "baidu_face.h"
#include "capture.h"
#include "json/json.h"
#include "base64.h"
#include "config.h"

using namespace std;

// libcurl库下载链接：https://curl.haxx.se/download.html
// jsoncpp库下载链接：https://github.com/open-source-parsers/jsoncpp/

std::string baidu_access_token;

const static std::string access_token_url = "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials";
const static std::string face_detect_url = "https://aip.baidubce.com/rest/2.0/face/v3/detect";
const static std::string add_face_url = "https://aip.baidubce.com/rest/2.0/face/v3/faceset/user/add";
const static std::string del_user_url = "https://aip.baidubce.com/rest/2.0/face/v3/faceset/user/delete";

static face_detect_info_t new_detect_info;

int face_set_detect_info(face_location_t *locat_array, int face_num)
{
    face_detect_info_t *detect = &new_detect_info;
    int i, j;

    for(i=0, j=0; i<face_num; i++)
    {
        if(locat_array[i].w < MIN_FACE_PIXEL || locat_array[i].h < MIN_FACE_PIXEL)
        {
            printf("face[%d] too samll, ignore ...\n", i);
            continue;
        }

        detect->locat[j] = locat_array[i];
        j++;
    }
    detect->face_num = j;

    return 0;
}

/* >0 人脸数量，<=0 失败，无人脸 */
int face_get_detect_info(face_location_t *locat_array, int array_size)
{
    int face_num = new_detect_info.face_num;

    if(new_detect_info.face_num <= 0)
        return -1;

    if(array_size < new_detect_info.face_num)
        face_num = array_size;

    for(int i=0; i<face_num; i++)
    {
        locat_array[i] = new_detect_info.locat[i];
    }

    return face_num;
}

void face_clear_detect_info(void)
{
    new_detect_info.face_num = 0;
}

/* https响应接收数据 */
inline size_t onWriteData(void * buffer, size_t size, size_t nmemb, void * userp)
{
    std::string * str = dynamic_cast<std::string *>((std::string *)userp);
    str->append((char *)buffer, size * nmemb);
    return nmemb;
}

static int json_parse_access_token(std::string json_str, std::string &access_token)
{
    // 开始获取json中的access token项目
    Json::Reader reader;
    Json::Value root;
    Json::Value json_access_token;
    bool bret;

    // 使用boost库解析json
    bret = reader.parse(json_str, root);
    if(!bret)
    {
        cout << "ERROR: parse json error!" << endl;
        return -1;
    }

    json_access_token = root["access_token"];
    if(json_access_token.isNull())
        return -1;

    access_token = json_access_token.asString();

    return 0;
}

static int json_parse_face_decect(std::string json_str, face_detect_info_t *detect_info)
{
    std::string err_code;
    Json::Reader reader;
    Json::Value root;
    bool bret;

    cout << json_str << endl;
    bret = reader.parse(json_str, root);
    if(!bret)
    {
        cout << "ERROR: parse json error!" << endl;
        return -1;
    }

    err_code = root["error_code"].asString();

    if(err_code == "0")
    {
        Json::Value json_result = root["result"];

        std::string str_facenum = json_result["face_num"].asString();
        //cout << "face_num: " << str_facenum << endl;
        int size = json_result["face_list"].size();     // array size
        detect_info->face_num = size;
        if(detect_info->face_num > MAX_FACE_NUM)
            detect_info->face_num = MAX_FACE_NUM;

        for(int i=0; i<size; i++)
        {
            Json::Value json_face_list = json_result["face_list"][i];

            Json::Value json_location = json_face_list["location"];
            std::string str_left = json_location["left"].asString();
            std::string str_top = json_location["top"].asString();
            std::string str_width = json_location["width"].asString();
            std::string str_height = json_location["height"].asString();

            detect_info->locat[i].x = atoi(str_left.c_str());
            detect_info->locat[i].y = atoi(str_top.c_str());
            detect_info->locat[i].w = atoi(str_width.c_str());
            detect_info->locat[i].h = atoi(str_height.c_str());
        }

    }
    else
    {
        detect_info->face_num = 0;
        return -1;
    }

    return 0;
}

static int json_parse_add_face(std::string json_str, bool &result)
{
    std::string err_code;
    Json::Reader reader;
    Json::Value root;
    bool bret;

    cout << json_str << endl;
    bret = reader.parse(json_str, root);
    if(!bret)
    {
        cout << "ERROR: parse json error!" << endl;
        return -1;
    }

    err_code = root["error_code"].asString();

    if(err_code == "0")
    {
        result = true;
    }
    else
    {
        result = false;
    }

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
    std::string access_token_str;
    CURL *curl;
    CURLcode result_code;
    int error_code = -1;

    curl = curl_easy_init();
    if (curl)
    {
        std::string url = access_token_url + "&client_id=" + AK + "&client_secret=" + SK;
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        std::string access_token_result;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &access_token_str);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWriteData);
        result_code = curl_easy_perform(curl);
        if (result_code != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));
            return -1;
        }

        json_parse_access_token(access_token_str, access_token);

        cout << "access token: " << access_token << endl;

        curl_easy_cleanup(curl);
        error_code = 0;
    } else
    {
        fprintf(stderr, "curl_easy_init() failed.");
        error_code = -1;
    }

    return error_code;
}

int baidu_add_user_face(const std::string access_token, const char *image_base64, const char *user_id)
{
    std::string data_str;
    std::string add_face_str;
    CURL *curl;
    CURLcode res;
    bool result = false;
    int ret;

    data_str = "{\"group_id\":\"";
    data_str += BAIDU_USER_GROUP;
    data_str += "\",\"image\":\"";
    data_str += image_base64;
    data_str += "\",\"image_type\":\"BASE64\",\"user_id\":\"";
    data_str += user_id;
    data_str += "\"}";

    curl = curl_easy_init();
    if(curl)
    {
        std::string url = add_face_url + "?access_token=" + access_token;
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data_str.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &add_face_str);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWriteData);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

            curl_easy_cleanup(curl);
            return -1;
        }
        cout << "add_face_str: " << add_face_str << endl;
        json_parse_add_face(add_face_str, result);
    }
    curl_easy_cleanup(curl);

    if(result == true)
        ret = 0;
    else
        ret = -1;

    return ret;
}

int baidu_delete_user(const std::string access_token, const char *user_id)
{
    std::string data_str;
    std::string result;
    CURL *curl;
    CURLcode res;

    data_str = "{\"group_id\":\"";
    data_str += BAIDU_USER_GROUP;
    data_str += "\",\"user_id\":\"";
    data_str += user_id;
    data_str += "\"}";

    curl = curl_easy_init();
    if(curl) 
    {
        std::string url = del_user_url + "?access_token=" + access_token;
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        //const char *data = "{\"group_id\":\"userGroup_1\",\"user_id\":\"123\"}";
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data_str.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWriteData);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

            curl_easy_cleanup(curl);
            return -1;
        }
        std::cout<<result;
    }
    curl_easy_cleanup(curl);

    return 0;
}

/**
 * 人脸检测与属性分析
 * @return 调用成功返回0，发生错误返回其他错误码
 */
int baidu_face_detect(face_detect_info_t *detect_info, const std::string access_token, const char *image_base64)
{
    std::string url = face_detect_url + "?access_token=" + access_token;
    std::string detect_result_str;
    std::string image_data;
    CURL *curl = NULL;
    CURLcode result_code;

    image_data = "{\"image\":\"";
    image_data += image_base64;
    image_data += "\",\"image_type\":\"BASE64\"}";

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, image_data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &detect_result_str);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onWriteData);
        result_code = curl_easy_perform(curl);
        if (result_code != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(result_code));

            curl_easy_cleanup(curl);
            return -1;
        }

        json_parse_face_decect(detect_result_str, detect_info);

        curl_easy_cleanup(curl);
    }
    else
    {
        fprintf(stderr, "curl_easy_init() failed.");
        return -1;
    }

    return 0;
}


int baidu_face_init(void)
{
    int ret;

    ret = baidu_get_access_token(baidu_access_token, BAIDU_API_KEY, BAIDU_SECRET_KEY);
    if(ret != 0)
    {
        printf("%s failed!\n", __FUNCTION__);
        return -1;
    }

    printf("%s ok.\n", __FUNCTION__);

    return 0;
}

unsigned char image_data[FRAME_BUF_SIZE];
unsigned char image_base64[IMAGE_BASE64_SIZE];

void *baidu_face_thread(void *arg)
{
    face_detect_info_t detect_info;
    static int old_frame_index = 0;
    int image_len;
    int base64_len;
    int ret;

    (void)arg;

    baidu_face_init();

    while(1)
    {
        ret = capture_get_newframe((unsigned char *)image_data, FRAME_BUF_SIZE, &image_len);
        if(ret <=0 || ret == old_frame_index)
        {
            //printf("%s: no new frame.\n", __FUNCTION__);
            usleep(100 *1000);
            continue;
        }
        old_frame_index = ret;

        base64_encode(image_data, image_len, image_base64, &base64_len);

        //printf("detect begin ...\n");
        ret = baidu_face_detect(&detect_info, baidu_access_token, (char *)image_base64);
        //printf("detect over, face num: %d\n", detect_info.face_num);
        if(ret==0 && detect_info.face_num>0)
        {
            face_set_detect_info(detect_info.locat, detect_info.face_num);
            for(int i=0; i<detect_info.face_num; i++)
            {
                printf("face[%d]: x=%d, y=%d, w=%d, h=%d\n", i, detect_info.locat[0].x, detect_info.locat[0].y, detect_info.locat[0].w, detect_info.locat[0].h);
            }
            //printf("baidu detect face succeess.\n");

        }

        ret = baidu_add_user_face(baidu_access_token, (char *)image_base64, "234");
        cout << "add face ret: " << ret << endl;

        baidu_delete_user(baidu_access_token, "123");

        sleep(3);
    }

    return NULL;
}

int start_baidu_face_task(void)
{
    pthread_t tid;
    int ret;

    ret = pthread_create(&tid, NULL, baidu_face_thread, NULL);
    if(ret != 0)
    {
        return -1;
    }

    return 0;
}

