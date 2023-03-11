#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


#define BACKGROUND_IMAGE    "../baidu_face/baidu_bg.png"

#define TIMER_DISPLAY_INTERV_MS			10

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void window_display(void);

private:
    Ui::MainWindow *ui;

    QTimer  *display_timer;				// 用于刷新显示
    unsigned char videobuf[FRAME_BUF_SIZE];      // 用于缓存视频图像

};


int mainwindow_init(void);

#endif // MAINWINDOW_H
