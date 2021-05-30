#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>

#define HAVE_OPENCV_IMGPROC
#define HAVE_OPENCV_CORE
#define HAVE_OPENCV_HIGHGUI

#include <opencv2/opencv.hpp>

//#include <opencv2/core.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>


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

    cv::destroyAllWindows();
}

/*
//    Cчитываем картинку
    cv::Mat img = cv::imread("./left13.jpg", cv::IMREAD_COLOR);

//    Cоздаём OpenCV-окно с именем "Original". Созданное окно  - контейнер,
//        его можно переиспользовать, расзмещать в нём виджеты OpenCV.
    cv::namedWindow("Original");

//    Показываем OpenCV-окно с именем "Original".  Если окна не существует - создаёт.
    cv::imshow("Original", img);

    Читать документацию, переходя к объявлению функции клавишей F2 !!!
*/


void MainWindow::on_pushButton_readim_clicked()
{
    /*
     * Окна и считывание изображений
     * Путь ../../data/squeezing.jpg значит что относительно текущего каталога,
     * где находится программа (исполняемый файл ЕХЕ) нужно поднятся на два
     * уровня выше (../../), затем зайти в папку data и открыть файл squeezing.jpg
    */
    cv::Mat img = cv::imread("../../data/squeezing.jpg", cv::IMREAD_COLOR);
    cv::namedWindow("Отжимания", cv::WINDOW_NORMAL);
    cv::imshow("Отжимания", img);
}

void MainWindow::on_pushButton_readasgray_clicked()
{
    /*
     * Окна и считывание изображений
    */
    cv::Mat img = cv::imread("../../data/dota1.jpg", cv::IMREAD_GRAYSCALE);
    cv::namedWindow("Gray");
    cv::imshow("Gray", img);
    /* Упражнение:
     * - выяснить чем отличается
     *    cv::namedWindow("Original", cv::WINDOW_NORMAL);
     * от
     *    cv::namedWindow("Original");
     *
     * - Изменить программу так, чтобы был вывод окна с цветными и серыми изображениями,
     *   уменьшенными в 2, 4, 8 разы;
    */
}

void MainWindow::on_pushButton_chng_bch_clicked()
{
    /*
     * Простые операции - эффекты над изображениями
    */
    cv::Mat im_src = cv::imread("../../data/dota1.jpg", cv::IMREAD_REDUCED_COLOR_2);
    cv::Mat img = im_src.clone();
    cv::Mat imgEff;
    cv::cvtColor(im_src, imgEff, cv::COLOR_BGR2GRAY);

    cv::imshow("Color", img);

    img += cv::Scalar(50,50,50);
    cv::imshow("Bright", img);
    img += cv::Scalar(0,0,50);
    cv::imshow("Add Red", img);

    // убираем предыдущие изменения
    img -= cv::Scalar(50,50,50);
    img -= cv::Scalar(0,0,50);

//    img *= 1.7;
    cv::imshow("Contrast", img * 1.7);

    cv::imshow("Grey", imgEff);
    imgEff += cv::Scalar(50,50,50);
    cv::imshow("Grey Bright", imgEff);

    imgEff -= cv::Scalar(50,50,50);
    imgEff *= 1.7;
    cv::imshow("Grey Contrast", imgEff);

    imgEff = im_src.clone();
    imgEff = cv::Scalar(255, 255, 255) - imgEff;
    cv::imshow("Color invert", imgEff);

    /* Упражнения:
     * - что будет, если изменять значение коэффициента, на который умножаем/прибавляем?
     * - что будет, если прибавить число больше 300
     * - что будет если умножать на отрицательные числа/ числа меньше 1?
     * - сделать ползунок, который будет изменять яркость изображения
     * - что такое cv::Scalar? Как изменить отдельный компонет cv::Scalar? 
     * - если cv::Scalar хранит описание  цветовой компоненты (красный, синий или зелёный), 
     * то какое наименьшее и наибольшее значение будут хранить элементы cv::Scalar ?
     * - сделать ползунки, изменяющие отдельно каждый цветовой канал изображения
     * - написать код, который сохраняет изменённые изображения в папку ./out
    */
}

// - - - - --------------------------------------------------------------------
//ползунок, который изменяет контрастность изображения
cv::Mat im_cntrst;
float thr_cntrst;

void MainWindow::on_pushButton_clicked()
{
    im_cntrst = cv::imread("../../data/dota1.jpg", cv::IMREAD_REDUCED_COLOR_2);
    cv::imshow("Contrast threshold", im_cntrst);
}

void MainWindow::on_dial_contrast_valueChanged(int value)
{
    thr_cntrst = (float) value/64;
    ui->label_contrast->setText(QString::number(thr_cntrst));
    cv::imshow("Contrast threshold", im_cntrst * thr_cntrst);
}
// - - - - --------------------------------------------------------------------

void MainWindow::on_pushButton_sum_im_clicked()
{
    /*
     * Простые операции - эффекты над изображениями
    */
    cv::Mat im1 = cv::imread("../../data/dota1.jpg", cv::IMREAD_COLOR);
    cv::Mat im2 = cv::imread("../../data/dota2.jpg", cv::IMREAD_COLOR);
    cv::Mat im_sum = im2 + im1;
    cv::imshow("Sum", im_sum);
    /* Упражнение:
     * - Повторить этот эффект с картинкой valve.png
     * - Проверить, как влияет изменение контраста/яркости одного изображения на
     * результат сложения двух изображений
     * - сохранить финальное изображение в папку ./out
     * -- сделать ползунок, регулирующий качество сохранённого изображения
    */
}

// ----------------------------------------------------------------------------
// Решение упражнения: ползунки, изменяющие отдельно каждый цветовой канал изображения

// переменные

void MainWindow::on_pushButton_load_thr_clicked()
{
    // загрузка и вывод исходного изображения
}

void MainWindow::on_dial_b_valueChanged(int value)
{
    // измение Синего канала
}

void MainWindow::on_dial_g_valueChanged(int value)
{
    // измение Зилёного канала
}

void MainWindow::on_dial_r_valueChanged(int value)
{
    // измение Красного канала
}

// ----------------------------------------------------------------------------
// Решение упражнения: сохранение изменений в файле
void MainWindow::on_pushButton_2_clicked()
{
    //
}
