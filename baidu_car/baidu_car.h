#ifndef BAIDU_CAR_H
#define BAIDU_CAR_H

#define LICENSE_NUM_SIZE    32

typedef struct {
    int x;
    int y;
    int w;
    int h;
}plate_location_t;

typedef struct {
    char license[LICENSE_NUM_SIZE];
    plate_location_t locat;
}license_plate_info_t;


int car_set_license_info(license_plate_info_t *license_info);
int car_get_license_info(license_plate_info_t *license_info);
void car_clear_license_info(void);

int start_baidu_car_task(void);


#endif // BAIDU_CAR_H
