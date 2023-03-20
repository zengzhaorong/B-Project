#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "capture.h"
#include "baidu_face.h"
#include "user_db.h"
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

    user_db_init(sql_db);

    face_rect_delay = FACE_RECT_DELAY_TIME;

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
    static face_location_t location[MAX_FACE_NUM];
    static int old_frame_index = 0;
    static int face_num = 0;
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

        // 在图像中，框出人脸
        ret = face_get_detect_info(location, MAX_FACE_NUM);
        if(ret > 0)
        {
            face_num = ret;
            face_rect_delay = 0;
            face_clear_detect_info();   // 清除检测的人脸
        }

        if(face_rect_delay < FACE_RECT_DELAY_TIME)
        {
            for(int i=0; i<face_num; i++)
            {
                QPainter painter(&videoQImage);
                painter.setPen(QPen(Qt::green, 3, Qt::SolidLine, Qt::RoundCap));
                painter.drawRect(location[0].x, location[0].y, location[0].w, location[0].h);
            }
            face_rect_delay ++;
            printf("face[%d], %d\n", face_num, face_rect_delay);
        }

        // 显示一帧图像
        ui->videoLab->setPixmap(QPixmap::fromImage(videoQImage));
        ui->videoLab->show();
    }

    display_timer->start(TIMER_DISPLAY_INTERV_MS);
}



void MainWindow::on_addUserBtn_clicked()
{
    QString str_id;
    QString str_name;
    QString str_user;
    int id;
    char name[32] = {0};

    str_id = ui->userIdEdit->text();
    str_name = ui->userNameEdit->text();
    if(str_id.length()<=0 || str_name.length()<=0)
    {
        qDebug() << "user id or name is null!";
        return ;
    }

    // toLocal8Bit(): Unicode编码
    id = atoi(str_id.toLocal8Bit().data());
    if(id <= 0)
    {
        qDebug() << "user id is illegal!";
        return ;
    }

    strcpy(name, str_name.toLocal8Bit().data());

    str_user = str_id + QString(", ") +str_name;
    qDebug() << "user: " << str_user;

    ui->userListBox->addItem(str_user);

}

void MainWindow::on_delUserBtn_clicked()
{
    QString str_user;
    int index;
    int id;

    str_user = ui->userListBox->currentText();
    if(str_user.length() <= 0)
    {
        qDebug() << "user is null!";
        return ;
    }

    id = atoi(str_user.toLocal8Bit().data());
    qDebug() << "del car user: " << str_user << ", id: " << id;

    index = ui->userListBox->currentIndex();
    ui->userListBox->removeItem(index);
}

void MainWindow::on_historyBtn_clicked()
{

}

/* main window initial - 主界面初始化 */
int mainwindow_init(void)
{
    mainwindow = new MainWindow;

    mainwindow->show();

    return 0;
}

