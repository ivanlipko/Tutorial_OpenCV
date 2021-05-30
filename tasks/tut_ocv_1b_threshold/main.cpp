#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat img;
Mat img_hsv;

const int max_thresh = 255;

int hMin = 29; int hMax = 79;
int sMin = 28; int sMax = 219;
int vMin = 77; int vMax = 255;

int cSize = 5;
int cArea = 800;

const char* window_source = "Source";
const char* window_thr = "Threshold";
const char* window_range = "Range";
const char* window_out = "Final";
void thresh_fun(int, void*);

const Scalar color = Scalar(0,0,255);    // BGR цвет контура

int main()
{
    img = imread("./num.jpg");
    if( img.empty() )
    {
        std::cout << "Image not loaded \n";
        return -1;
    }
    imshow(window_source, img);

    cvtColor(img, img_hsv, COLOR_BGR2HSV);
    imshow(window_range, img);

    namedWindow(window_thr);
    // HSV фильтр
    createTrackbar("hMin", window_thr, &hMin, max_thresh, thresh_fun);
    createTrackbar("hMax", window_thr, &hMax, max_thresh, thresh_fun);
    createTrackbar("sMin", window_thr, &sMin, max_thresh, thresh_fun);
    createTrackbar("sMax", window_thr, &sMax, max_thresh, thresh_fun);
    createTrackbar("vMin", window_thr, &vMin, max_thresh, thresh_fun);
    createTrackbar("vMax", window_thr, &vMax, max_thresh, thresh_fun);
    // Фильтр размеровконтура
    createTrackbar("Size", window_thr, &cSize, 50, thresh_fun);
    createTrackbar("Area", window_thr, &cArea, 1500, thresh_fun);

    thresh_fun(0, 0);
    waitKey();

    return 0;
}

void thresh_fun(int, void*)
{
    // Бинаризуем изображение (делаем его чёрно-белым)
    Mat img_range;
    Scalar lower(hMin, sMin, vMin);
    Scalar upper(hMax, sMax, vMax);
    inRange(img_hsv, lower, upper, img_range);
    imshow(window_range, img_range);

    // находим контуры
    vector<vector<Point> > contours;
    findContours(img_range, contours, RETR_TREE, CHAIN_APPROX_NONE);

    // Результирующее изображение
    Mat drawing;
    img.copyTo(drawing);

    //    Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
    std::cout << contours.size() <<std::endl;

    for (unsigned int i = 0; i < contours.size(); i++) {  // проходим по всем контурам, которые нашли
        //        std::cout << "Size" << contours.at(i).size() << "Area " << contourArea(contours.at(i));
        if (contours.at(i).size() < cSize)     // если этот контур короткий (мало точек), то
            continue;                        // пропускаем его и переходим к следующей итерации
        if (fabs(contourArea(contours.at(i))) < cArea)   // если этот контур с маленькой площадью, то
            continue;							// пропускаем его и переходим к следующей итерации

        std::cout << "Size" << contours.at(i).size() << " Area " << contourArea(contours.at(i)) << ";  ";
        // а если это подхдоящий по размерам контур, то:

        // 1. ИЛИ описываем эллипс вокруг контуров (или можно описывать вокруг аппроксимации, как ниже по коду)
        RotatedRect bEllipse = fitEllipse(contours.at(i));
        // и рисуем его
        ellipse(drawing, bEllipse, Scalar(255, 0, 0), 2, 8);


        // 2. ИЛИ
        // аппроксимируем его
        vector<vector<Point> > contours_poly(contours.size());
        approxPolyDP(contours[i], contours_poly[i], 3, true);

        // рисуем аппроксимровнные контуры на изображении
        drawContours(drawing, contours_poly, i, color);

        // 2.A. строим вокруг контура ограничивающий прямоуглоьник
        //*
        vector<Rect> boundRect(contours.size());
        boundRect[i] = boundingRect(contours_poly[i]);
        // рисуем на изображении описанный прямоугольник
        rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 2);
        //*/
        // 2.B. ИЛИ
        // находим центр и радиус описанной окружности
        //*
        vector<Point2f> centers(contours.size());
        vector<float> radius(contours.size());
        minEnclosingCircle(contours_poly[i], centers[i], radius[i]);
        // рисуем на изображении описанную окружность
        circle(drawing, centers[i], (int)radius[i], color, 2);
        //*/
    }

    // Выводим результирующий рисунок
    imshow(window_out, drawing);

}



