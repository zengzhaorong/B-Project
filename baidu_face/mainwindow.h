#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


#define BACKGROUND_IMAGE    "../baidu_face/baidu_bg.png"

#define TIMER_DISPLAY_INTERV_MS			1
#define FACE_RECT_DELAY_TIME      10

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QSqlDatabase sql_db;

private slots:
    void window_display(void);

    void on_addUserBtn_clicked();

    void on_delUserBtn_clicked();

    void on_historyBtn_clicked();

private:
    Ui::MainWindow *ui;

    QTimer  *display_timer;				// 用于刷新显示
    unsigned char videobuf[FRAME_BUF_SIZE];      // 用于缓存视频图像
    int face_rect_delay;    // 用人脸框显示延时
};


int mainwindow_init(void);

#endif // MAINWINDOW_H
