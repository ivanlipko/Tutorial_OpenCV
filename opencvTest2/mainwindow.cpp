#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
        qDebug() << 0;
    ui->setupUi(this);
    qDebug() << 1;
    // read an image
    cv::Mat image = cv::imread("D://left13.jpg", 1);
    qDebug() << 2;
    // create image window named "My Image"
    cv::namedWindow("My Image");
    // show the image on window
    cv::imshow("My Image", image);
}

MainWindow::~MainWindow()
{
    delete ui;
}
