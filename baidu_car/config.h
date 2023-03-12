#ifndef CONFIG_H
#define CONFIG_H



// [WINDOW]
#define DEFAULT_WINDOW_TITLE		"百度AI车牌识别系统"

// [CAPTURE]
#define DEFAULT_CAPTURE_DEV 		"/dev/video0"
#define DEFAULT_CAPTURE_WIDTH		640
#define DEFAULT_CAPTURE_HEIGH		480
#define FRAME_BUF_SIZE		(DEFAULT_CAPTURE_WIDTH *DEFAULT_CAPTURE_HEIGH *3)
#define IMAGE_BASE64_SIZE   ((FRAME_BUF_SIZE/3+1)*4)

#endif // CONFIG_H
