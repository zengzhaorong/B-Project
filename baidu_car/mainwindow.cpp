#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "capture.h"
#include "config.h"


static MainWindow *mainwindow;


/*  将jpge/mjpge格式转换为QImage */
QImage jpeg_to_QImage(unsigned char *data, int len)
{
    QImage qtImage;

    if(data==NULL || len<=0)
        return qtImage;

    qtImage.loadFromData(data, len);
    if(qtImage.isNull())
    {
        printf("ERROR: %s: QImage is null !\n", __FUNCTION__);
    }

    return qtImage;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* set window title - 设置窗口标题 */
    setWindowTitle(DEFAULT_WINDOW_TITLE);

    /* 加载并显示背景图 */
    QImage image;
    image.load(BACKGROUND_IMAGE);
    ui->videoLab->setPixmap(QPixmap::fromImage(image));

    display_timer = new QTimer(this);
    connect(display_timer, SIGNAL(timeout()), this, SLOT(window_display()));
    display_timer->start(TIMER_DISPLAY_INTERV_MS);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::window_display(void)
{
    static int old_frame_index = 0;
    QImage videoQImage;
    int frame_len = 0;
    int ret;

    display_timer->stop();

    // 获取最新一帧图像
    ret = capture_get_newframe(videobuf, FRAME_BUF_SIZE, &frame_len);
    if(ret > 0 && ret != old_frame_index)
    {
        old_frame_index = ret;
        //qDebug() << "frame index " << ret;
        videoQImage = jpeg_to_QImage(videobuf, frame_len);

        // 显示一帧图像
        ui->videoLab->setPixmap(QPixmap::fromImage(videoQImage));
        ui->videoLab->show();
    }

    display_timer->start(TIMER_DISPLAY_INTERV_MS);
}


/* main window initial - 主界面初始化 */
int mainwindow_init(void)
{
    mainwindow = new MainWindow;

    mainwindow->show();

    return 0;
}

