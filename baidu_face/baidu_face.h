#ifndef BAIDU_FACE_H
#define BAIDU_FACE_H

#define MIN_FACE_PIXEL  100     // 最小人脸像素，小于过滤丢掉
#define MAX_FACE_NUM    10

#define BAIDU_USER_GROUP      "userGroup_1"

typedef struct {
    int x;
    int y;
    int w;
    int h;
}face_location_t;

typedef struct {
    int face_num;
    face_location_t locat[MAX_FACE_NUM];
}face_detect_info_t;


int face_set_detect_info(face_location_t *locat_array, int face_num);
int face_get_detect_info(face_location_t *locat_array, int array_size);
void face_clear_detect_info(void);


int start_baidu_face_task(void);


#endif // BAIDU_FACE_H
