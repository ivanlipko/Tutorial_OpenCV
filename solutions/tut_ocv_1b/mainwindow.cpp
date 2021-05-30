#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QMessageBox>

#include <opencv2/opencv.hpp>

/*
 * Обрати внимание что подключается в проектоном файле!
 * Там должна быть залинкована библа.
*/

/*
    Читать документацию, переходя к объявлению функции клавишей F2 !!!

    Графика:
        https://docs.opencv.org/4.0.1/d6/d6e/group__imgproc__draw.html
    Thresholding, определение порогов:
        https://docs.opencv.org/master/db/d8e/tutorial_threshold.html
        https://docs.opencv.org/master/da/d97/tutorial_threshold_inRange.html
    Цветовое выделение:
        https://docs.opencv.org/4.0.1/df/d9d/tutorial_py_colorspaces.html
    Наращивание и вымывание пикселей:
        https://docs.opencv.org/master/db/df6/tutorial_erosion_dilatation.html

    Контуры:
        https://docs.opencv.org/master/df/d0d/tutorial_find_contours.html
        https://docs.opencv.org/master/da/d0c/tutorial_bounding_rects_circles.html
        https://docs.opencv.org/master/de/d62/tutorial_bounding_rotated_ellipses.html
        (**)
        https://docs.opencv.org/master/d0/d49/tutorial_moments.html

    Таймер Qt:
        https://doc.qt.io/QT-5/qtimer.html#details
    Фильтрация изображений:
        https://docs.opencv.org/master/dc/dd3/tutorial_gausian_median_blur_bilateral_filter.html

*/

cv::VideoCapture cam(0);  // Вебкамера

cv::Scalar hsv_low(0,0,0), hsv_high(255,255,255);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDir dir;
    qDebug() << dir.currentPath();
}

MainWindow::~MainWindow()
{
    delete ui;

    timer->stop();
    cam.release();
    delete timer;
    cv::destroyAllWindows();
}

void MainWindow::on_pushButton_run_clicked()
{
    if(!cam.isOpened()) {
        QMessageBox::warning(this, "Error", "Camera not init");
        return;
    }

    cv::Size camSize = cv::Size((int) cam.get(cv::CAP_PROP_FRAME_WIDTH),
                                (int) cam.get(cv::CAP_PROP_FRAME_HEIGHT));
    ui->label_cam_info->setText( QString::number(camSize.width) + "x"
                                  + QString::number(camSize.height));

    // Создаём таймер, который будет спрашивать кадры
    if (timer == NULL) {
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(nextFrame()));
        timer->start(1000/cam.get(cv::CAP_PROP_FPS));
    }
}

void MainWindow::on_pushButton_stop_clicked()
{
    // stop camera reading and erase it
    timer->stop();
    delete timer;
    timer = NULL;
//    cv::destroyWindow("Source");
}

void MainWindow::nextFrame()
{
    cv::Mat frame;
    cam.read(frame);

    cv::Mat frame_hsv;
    cv::cvtColor(frame, frame_hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    cv::inRange(frame_hsv, hsv_low, hsv_high, mask);

    /*
     * Упражнения:
     * - добавить erode, затем dilate. Что изменилось? чем это может быть полезно?
     * (**) - подумайте, как можно реализовать выделение нескольких объектов
     * разных цветов?
    */

    cv::Mat res;
    cv::bitwise_and(frame, frame, res, mask);

    cv::imshow("Source", frame);
    cv::imshow("Mask", mask);
    //    cv::imshow("HSV", frame_hsv);
    cv::imshow("Source + Mask", res);

    //
    // Поиск и выделение контуров
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours( mask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE );

    std::vector<std::vector<cv::Point> > contours_poly( contours.size() );     // аппроксимарованные ломаные
    std::vector<cv::Rect> boundRect( contours.size() );
    std::vector<cv::Point2f>centers( contours.size() );
    std::vector<float>radius( contours.size() );

    // Для каждой ломанной применяем аппроксимацию (упрощение) с точностью +-3,
    //    и указываем, чтобы она обязательно была замкнутой. После этого вокруг
    //    каждого полигона описываем прямоугольник и окружность.
    // Из описанной окружности выделяем её центр и радиус - это позволит нам
    //    нарисовать его.
    for( size_t i = 0; i < contours.size(); i++ ) {
        cv::approxPolyDP( contours[i], contours_poly[i], 3, true );
        boundRect[i] = cv::boundingRect( contours_poly[i] );
        cv::minEnclosingCircle( contours_poly[i], centers[i], radius[i] );
    }

    /*
     * Упражнения:
     * - сделать ограничение на размер найденного контура. Если он меньше
     * заданного числа К, то не обрабатывать его вовсе
     * - задавать К с помощью слайдера (диапазон находится экспериментально)
    */

    cv::RNG rng;
    for( unsigned int i = 0; i< contours.size(); i++ ) {
        cv::Scalar color = cv::Scalar( rng.uniform(0, 256), rng.uniform(0,256), rng.uniform(0,256) );
        cv::drawContours( frame, contours_poly, i, color );
        cv::rectangle( frame, boundRect[i].tl(), boundRect[i].br(), color, 2 );
        cv::circle( frame, centers[i], (int)radius[i], color, 2 );
    }
    cv::imshow("Countours", frame);


    /*
     * Упражнения:
     * - сделать описывание найденных контуров эллипсами
     * - сделать описывание найденных контуров повернутыми прямоугольниками
     * - сделать вывод в окне текста, содержащего угол наклона прямоугольника
     * (**) - попробуйте использовать детектор границ Кани (функция cv::Canny).
     * Возможно он будет полезен в некоторых приложениях.
     * (**) - повторите урок с вычислениями моментов. Это может быть полезно
     * когда необходимо выделить несколько объектов разных цветов
    */
}

void MainWindow::on_verticalSlider_hsv_low_h_valueChanged(int value)
{
    hsv_low = cv::Scalar(value, hsv_low[1], hsv_low[2]);
    setGBColor();
}

void MainWindow::on_verticalSlider_hsv_high_h_valueChanged(int value)
{
    hsv_high = cv::Scalar(value, hsv_high[1], hsv_high[2]);
}

void MainWindow::on_verticalSlider_hsv_low_s_valueChanged(int value)
{
    hsv_low = cv::Scalar(hsv_low[0], value, hsv_low[2]);
    setGBColor();
}

void MainWindow::on_verticalSlider_hsv_high_s_valueChanged(int value)
{
    hsv_high = cv::Scalar(hsv_high[0], value, hsv_high[2]);
}

void MainWindow::on_verticalSlider_hsv_low_v_valueChanged(int value)
{
    hsv_low = cv::Scalar(hsv_low[0], hsv_low[1], value);
    setGBColor();
}

void MainWindow::on_verticalSlider_hsv_high_v_valueChanged(int value)
{
    hsv_high = cv::Scalar(hsv_high[0], hsv_high[1], value);
}

void MainWindow::setGBColor()
{
//    cv::Scalar rgb;
//    cv::cvtColor(hsv_low, rgb, cv::COLOR_HSV2BGR);
//    ui->groupBox_inrange->setStyleSheet("background-color: rgb("+
//        QString::number(rgb[0])+","+ QString::number(rgb[1])+","+
//        QString::number(rgb[2])+") ");

    /*
     * Упражнение:
     * - попробуйте реализовать раскрашивание виджета в цвет, который выбран
     * слайдерами
    */
}
