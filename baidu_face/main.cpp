#include "mainwindow.h"
#include <QApplication>
#include <unistd.h>
#include "capture.h"
#include "baidu_face.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 初始化显示界面
    mainwindow_init();

    // 设置stdout缓冲大小为0，使得printf马上输出
    setbuf(stdout, NULL);

    newframe_mem_init();

    sleep(1);	// only to show background image
    start_capture_task();

    start_baidu_face_task();

    return a.exec();
}
