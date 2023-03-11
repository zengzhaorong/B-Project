#ifndef CONFIG_H
#define CONFIG_H

/******************** default configure value ********************/

typedef unsigned int		uint32_t;
typedef signed int			int32_t;

typedef unsigned short		uint16_t;
typedef signed short		int16_t;

typedef unsigned char 		uint8_t;
typedef signed char 		int8_t;


// [WINDOW]
#define DEFAULT_WINDOW_TITLE		"百度AI人脸识别系统"

// [CAPTURE]
#define DEFAULT_CAPTURE_DEV 		"/dev/video0"
#define DEFAULT_CAPTURE_WIDTH		640
#define DEFAULT_CAPTURE_HEIGH		480
#define FRAME_BUF_SIZE		(DEFAULT_CAPTURE_WIDTH *DEFAULT_CAPTURE_HEIGH *3)
#define IMAGE_BASE64_SIZE   ((FRAME_BUF_SIZE/3+1)*4)

// [BAIDU AI]
#define BAIDU_API_KEY       "osdH6GrOh2dCDDZSjLfnI4Rl"
#define BAIDU_SECRET_KEY    "u8FTpnjWYkeLGyghYfcnfNvjiPfEW6AG"



#endif // CONFIG_H
