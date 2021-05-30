#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QMessageBox>

#define HAVE_OPENCV_CORE
#define HAVE_OPENCV_IMGPROC
#define HAVE_OPENCV_HIGHGUI
#define HAVE_OPENCV_VIDEOIO

#include <opencv2/opencv.hpp>

//#include <opencv2/videoio.hpp>
//#include <opencv2/core.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>

/*
 * Обрати внимание что подключается в проектоном файле!
 * Там должна быть залинкована библа videoio.
*/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDir dir;
    qDebug() << dir.currentPath();
}

/*
    Читать документацию, переходя к объявлению функции клавишей F2 !!!
    Графика:
        https://docs.opencv.org/4.0.1/d6/d6e/group__imgproc__draw.html
    Видео:
        https://docs.opencv.org/4.0.1/d5/dc4/tutorial_video_input_psnr_ssim.html
        https://docs.opencv.org/4.0.1/d4/d15/group__videoio__flags__base.html
    Таймер Qt:
        https://doc.qt.io/QT-5/qtimer.html#details
    Фильтрация изображений:
        https://docs.opencv.org/master/dc/dd3/tutorial_gausian_median_blur_bilateral_filter.html

*/

void MainWindow::on_pushButton_readim_clicked()
{
    /*
     * рисуем фигуры
    */
    cv::Mat im = cv::Mat::zeros(cv::Size(900,600), CV_8UC3);
    cv::line(im, cv::Point(50,10), cv::Point(150,200), cv::Scalar(130,0,0));
    cv::line(im, cv::Point(150,300), cv::Point(50,150), cv::Scalar(130,200,0), 10);
    cv::line(im, cv::Point(200,50), cv::Point(150,200), cv::Scalar(130,0,250), 10);

    cv::ellipse(im, cv::Point(500, 250), cv::Size(150,100), 45, 0, 360, cv::Scalar(0,0,200));
    cv::ellipse(im, cv::Point(600, 150), cv::Size(150,150), 45, 0, 180, cv::Scalar(150,0,200), cv::FILLED, cv::LINE_8);

    cv::circle(im, cv::Point(300, 400), 50, cv::Scalar(140, 150, 0));

    cv::rectangle(im, cv::Rect(650, 250, 90, 150), cv::Scalar(90,170,210), 4);

    cv::rectangle(im, cv::Rect(550, 350, 50, 75), cv::Scalar(90,220,110), cv::FILLED, cv::LINE_AA);

    cv::drawMarker(im, cv::Point(400,350), cv::Scalar(200,200,150));

    cv::String text = "Funny text inside the box";
    int fontFace = cv::FONT_HERSHEY_SCRIPT_SIMPLEX;
    double fontScale = 2;
    int thickness = 3;

    int baseline=0;
    cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
    baseline += thickness;

    // center the text
    cv::Point textOrg((im.cols - textSize.width)/2,
                      (im.rows + textSize.height)/2);

    // draw the box
    cv::rectangle(im, textOrg + cv::Point(0, baseline),
                  textOrg + cv::Point(textSize.width, -textSize.height),
                  cv::Scalar(0,0,255));
    // ... and the baseline first
    cv::line(im, textOrg + cv::Point(0, thickness),
             textOrg + cv::Point(textSize.width, thickness),
             cv::Scalar(0, 0, 255));

    // then put the text itself
    cv::putText(im, text, textOrg, fontFace, fontScale,
                cv::Scalar::all(255), thickness, 8);

    const uint64 seed = 150;
    cv::RNG rng(seed);
    for ( int i = 1; i < 50; i++ ) {
        cv::Point org;
        org.x = rng.uniform(im.rows/2, im.rows);
        org.y = rng.uniform(im.cols/2, im.cols);
        cv::putText( im, "Testing text rendering", org, rng.uniform(0,8),
                     rng.uniform(0,100)*0.05+0.1, cv::Scalar(0, 0, 255), rng.uniform(1, 10), cv::LINE_4);
    }

    cv::namedWindow("Draw", cv::WINDOW_NORMAL);
    cv::imshow("Draw", im);

    /* Упражнения:
     * - считать любое изображение и нарисовать на нём что-нибудь
     * - нарисовать закрашенный круг
     * - написать своё имя в правом верхнем углу прямым шрифтом. Имя и фамилия
     * разным размером букв
     * - расставить по углам разные маркеры
     * - нарисовать линию со cтрелкой
     * - сделать фон изображения белым (сейчас он чёрный)
     * - *(std) вывод введённого пользователем текста на изображение
     * (например, добавить в предыдущую программу работы с ползунками вывод
     * параметров с помощью функции текста OpenCV
    */
}

void MainWindow::on_pushButton_cam_clicked()
{
    cv::VideoCapture cam("../data/left.webm");
    //    cv::VideoCapture cam(0);  // Вебкамера
    if(!cam.isOpened()) {
        QMessageBox::warning(this, "Error", "Camera not init");
        return;
    }

    cv::Size camSize = cv::Size((int) cam.get(cv::CAP_PROP_FRAME_WIDTH),
                                (int) cam.get(cv::CAP_PROP_FRAME_HEIGHT));
    ui->label_videoSize->setText( QString::number(camSize.width) + "x"
                                  + QString::number(camSize.height));
    //

    cv::Mat frame;

    cam >> frame;
    // или
    cam.read(frame);

    cv::imshow("Cam", frame);

    /*
     * Упражнения:
     * - вывести информацию о видео: FPS, формат, гамму, яркость,
     * текущее время в видео (position), количество кадров (frame count)
    */
}

/*
 * Проигрываем видео
*/

//cv::VideoCapture cap("../data/movie.mov");
cv::VideoCapture cap(0);
void MainWindow::on_pushButton_movieStream_clicked()
{
    if(!cap.isOpened()) {
        QMessageBox::warning(this, "Error", "Camera not init");
        return;
    }

    // Создаём таймер, который будет спрашивать кадры
    if (timer == NULL) { // если таймера ещё не существует, то ...
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(nextFrame()));
        //    timer->start(1000); //  берём кадры раз в секунду.

        // запускаем и смотрим - слишком прерывисто.
        // А почему не узнать сколько у нас FPS и делать именно такую задержку?
        // FPS - Frames Per Second: количество кадров в 1 секунду
        // При FPS = 25, задержка между кадрами: 1 сек/25 кадров = 0,04 сек ~= 40 мсек
        timer->start(1000/cap.get(cv::CAP_PROP_FPS));

        ui->label_2->setText(QString::number(cap.get(cv::CAP_PROP_FRAME_COUNT)));
        ui->horizontalSlider->setMaximum((int) cap.get(cv::CAP_PROP_FRAME_COUNT));
    }
}

void MainWindow::nextFrame()
{
    cv::Mat frame;
    cap.read(frame);
    cv::cvtColor(frame, frame, cv::COLOR_BGR2HSV);
    cv::imshow("Video", frame);

    /*
     * Упражнение:
     * - предусмотреть, чтобы прога не вылетала, когда заканчивается видео.
    */
}

void MainWindow::on_pushButton_clicked()
{
    timer->stop();
    delete timer;
    timer = NULL;
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    //captRefrnc.set(CAP_PROP_POS_MSEC, 1.2);  // go to the 1.2 second in the video
    //captRefrnc.set(CAP_PROP_POS_FRAMES, 10); // go to the 10th frame of the video
    // now a read operation would read the frame at the set position
    cap.set(cv::CAP_PROP_POS_FRAMES, (double) value);
}

/*
 * Упражнение:
 * - повторить урок https://docs.opencv.org/4.0.1/d5/dc4/tutorial_video_input_psnr_ssim.html
*/


/* ----------------------------------------------------------------------------
 * Фильтрация изображения
 * ----------------------------------------------------------------------------
*/
void MainWindow::nextFrameFiltered()
{
    cv::Mat frame;
    cap.read(frame);

//    cv::GaussianBlur(frame, frame, cv::Size(15,15), 0);
    /*
     * Упражнение:
     * - выяснить, на что влияет размер Size?
     * - а если указывать не симметричный размер?
     * - что будет, если повторить фильтрацию с небольшим размером Size(3,3)?
     * чем это будет отличаться от большого размера фильтра?
    */

//    cv::medianBlur(frame, frame, 2);
    /*
     * Вопрос: чем отличается от гауссовского фильтра? что становится заметным,
     * а что менее заметным?
    */

//    cv::blur(frame, frame, cv::Size(5,5));
//    cv::boxFilter(frame, frame, 0.5, cv::Size(5,5));
    /*
     * Вопрос: чем отличается от предыдущих фильтров? что становится заметным,
     * а что менее заметным?
    */

    cv::Mat frameIn = frame.clone();
    cv::bilateralFilter(frameIn, frame, 15, 255, 10);

    cv::imshow("Video", frame);

    /*
     * Упражнение:
     * - применить гауссовский фильтр несколько раз, пояснить результат
     * - сделать для нескольких фильтров ползунки (скролбары) настройки параметров
     * - * повторить урок https://docs.opencv.org/master/da/d5c/tutorial_canny_detector.html
    */
}

void MainWindow::on_pushButton_runFilterVideo_clicked()
{
    /*
     * Упражнение:
     * - загрузить видео
     * - проигрывать его в таймере
     * - назначить по вызову таймера слот nextFrameFiltered()
     * - удалить лишний код и комментарии
    */

    if(!cap.isOpened()) {
        QMessageBox::warning(this, "Error", "Camera not init");
        return;
    }

    if (timer == NULL) {
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(nextFrameFiltered()));
        timer->start(1000/cap.get(cv::CAP_PROP_FPS));

        ui->label_2->setText(QString::number(cap.get(cv::CAP_PROP_FRAME_COUNT)));
        ui->horizontalSlider->setMaximum((int) cap.get(cv::CAP_PROP_FRAME_COUNT));
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    timer->stop();
    cap.release();
    delete timer;
    cv::destroyAllWindows();
}



void MainWindow::on_pushButton_readImtoFilter_clicked()
{
    cv::Mat im_src;
    cv::Mat im_clear;
    cv::Mat im;
    im_src = cv::imread("../data/star-noise2.jpg", cv::IMREAD_REDUCED_COLOR_2);
    im = im_src.clone();
    im_clear = cv::imread("../data/star.jpg", cv::IMREAD_REDUCED_COLOR_2);

    /*
     * Упражнение:
     * - максимально улучшить качество изображения с шумами.
    */
    cv::medianBlur(im_src, im, 3);

    cv::imshow("Clear Image", im_clear);
    cv::imshow("Source Image", im_src);
    cv::imshow("Filter Image", im);
}

