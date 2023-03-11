#ifndef BAIDU_FACE_H
#define BAIDU_FACE_H

#define MAX_FACE_NUM    10

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

int start_baidu_face_task(void);


#endif // BAIDU_FACE_H
