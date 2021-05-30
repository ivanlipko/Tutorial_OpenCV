#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QMap>

#define HAVE_OPENCV_CORE
#define HAVE_OPENCV_IMGPROC
#define HAVE_OPENCV_HIGHGUI
#define HAVE_OPENCV_ML
#define HAVE_OPENCV_OBJDETECT

#include <opencv2/opencv.hpp>

/*
 * Обрати внимание что подключается в проектном файле!
 * Там должно быть залинкованs библиотеки.
 * Обращай внимание на пути к файлам, чтобы загружать данные.
*/

/*
    Читать документацию, переходя к объявлению функции клавишей F2 !!!
        https://www.pyimagesearch.com/2014/01/27/hobbits-and-histograms-a-how-to-guide-to-building-your-first-image-search-engine-in-python/
    Построение гистограммы
        https://docs.opencv.org/4.0.1/d8/dbc/tutorial_histogram_calculation.html
    Сравнение гистограмм
        https://docs.opencv.org/4.0.1/d8/dc8/tutorial_histogram_comparison.html
    Std Vector
        https://ru.cppreference.com/w/cpp/container/vector


*/

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
}

/*
 * Упражнения:
 * - реализовать проверку по тестовым изображениям, а не из базы
 * - исследовать влияние параметров (bins, range, нормы) гистограмм,
 * размера изображений, нормализации и проч. на качество поиска
 * - исследовать влияние цветовых представлений BGR, HSV, HLS на качество поиска (повторить предыдущий пункт)
*/

// Размер гистограммы
const int hist_bins = 256;

struct TDbItem {
    QString fname ;
    float dist;
    cv::Mat im;
    cv::Mat hist;
};
std::vector< TDbItem> db_im;

size_t load_images( const cv::String & dirname, std::vector< TDbItem > & db_lst)
{
    std::vector< cv::String > files;
    cv::glob( dirname, files );

    for ( size_t i = 0; i < files.size(); ++i )  {
        cv::Mat img = cv::imread( files[i] ); // load the image
        if ( img.empty() ) {            // invalid image, skip it.
            std::cout << files[i] << " is invalid!" << std::endl;
            continue;
        }
        cv::resize(img,img, cv::Size(64,64));

        TDbItem item;
        item.fname = QString(files[i].c_str());
        item.im = img;
        item.dist = 0;

        db_lst.push_back(item);
    }

    return files.size();
}

void calc_hist(cv::Mat im, cv::Mat &res, const bool isShow = false){
    int histSize[] = { hist_bins, hist_bins, hist_bins };

    float bgr_ranges[] = { 0, 255 };
    const float* ranges[] = { bgr_ranges, bgr_ranges, bgr_ranges };

    int channels[] = { 0, 1, 2 };

    cv::calcHist( &im, 1, channels, cv::Mat(), res, 2, histSize, ranges, true, false );
    cv::normalize( res, res, 0, 1, cv::NORM_MINMAX, -1, cv::Mat() );

    /*
    // достаём цветовые каналы из изображения
    std::vector <cv::Mat> channels;
    cv::split(im, channels);

    const int histSize = res.rows / 3;

    //    const bool uniform = true, accumulate = false;
    float range[] = { 0, 256 }; //the upper boundary is exclusive
    const float* histRange = { range };
    cv::Mat b_hist, g_hist, r_hist;

    // гистограмма по каждому каналу
    cv::calcHist (&channels[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange); //, uniform, accumulate  )
    cv::calcHist (&channels[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange); //, uniform, accumulate  )
    cv::calcHist (&channels[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange); //, uniform, accumulate  )

        cv::normalize(b_hist, b_hist, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
        cv::normalize(g_hist, g_hist, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
        cv::normalize(r_hist, r_hist, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

//    // Get dimension of final image
//    int rows = histSize * 3;
//    int cols = 1;

//    // Create a black image
//    res(rows, cols, CV_8UC1, cv::Scalar(0,0,0));
    // Copy images in correct position
    b_hist.copyTo(res(cv::Rect(0, 0, 1, histSize)));
    g_hist.copyTo(res(cv::Rect(0, histSize, 1, histSize)));
    r_hist.copyTo(res(cv::Rect(0, 2*histSize, 1, histSize)));

    if (!isShow)
        return;
    // Show histogram
    const int hist_w = 512, hist_h = 400;
    cv::Mat im_hist( hist_h, hist_w, CV_8UC3, cv::Scalar(0,0,0) );
    int bin_w = cvRound( (double) hist_w/histSize );

    cv::normalize(b_hist, b_hist, 0, hist_h, cv::NORM_MINMAX, -1);
    cv::normalize(g_hist, g_hist, 0, hist_h, cv::NORM_MINMAX, -1);
    cv::normalize(r_hist, r_hist, 0, hist_h, cv::NORM_MINMAX, -1);

    for( int i = 1; i < histSize; i++ ) {
        cv::line( im_hist, cv::Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ),
                  cv::Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                  cv::Scalar( 255, 0, 0), 2, 8, 0  );
        cv::line( im_hist, cv::Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ),
                  cv::Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                  cv::Scalar( 0, 255, 0), 2, 8, 0  );
        cv::line( im_hist, cv::Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ),
                  cv::Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                  cv::Scalar( 0, 0, 255), 2, 8, 0  );
    }
    cv::namedWindow("calcHist", cv::WINDOW_NORMAL);
    cv::imshow("calcHist", im_hist );
//*/
}


bool compare_db_items (TDbItem A, TDbItem B)
{
    return (A.dist > B.dist);
}

void compare_images(QString path){
    std::string ppath = path.toStdString();
    cv::Mat test_im = cv::imread(ppath);
    cv::Mat test_hist;

    // Считаем гистограмму
    calc_hist(test_im, test_hist, true);

    // Сравниваем гистограмму с теми, что есть в базе
    for(uint i=0; i < db_im.size(); i++){
        float dist = cv::compareHist(db_im[i].hist, test_hist, cv::HISTCMP_INTERSECT);

        db_im[i].dist = dist;
    }

    // Сортируем результат и выводим
    std::sort(db_im.begin(), db_im.end(), compare_db_items);

    const uint showCount = 8;
    uint rows,cols;
    rows = db_im[0].im.rows;
    cols = db_im[0].im.cols;
    cv::Mat merged(showCount * rows, cols, db_im[0].im.type());

    for(uint i=0; i < db_im.size(); i++){
        if (i < showCount){
            db_im[i].im.copyTo(merged(cv::Rect(0,rows*i,rows,cols)));
        }
    }

    // Смотрим на результат
    cv::imshow("test_im", test_im);
    cv::imshow("merged", merged);
}

void MainWindow::on_pushButton_clicked()
{
    qDebug() << "Load images... ";
    int count = load_images( "../../data/searcher/db", db_im);
    qDebug() << "Loaded " << count << " images";

    for(uint i=0; i< db_im.size(); i++){
        calc_hist(db_im[i].im, db_im[i].hist);

        ui->listWidget_dbfiles->addItem(db_im[i].fname);
    }
}

void MainWindow::on_pushButton_find_clicked()
{
    compare_images("../../data/searcher/db/sun_6.jpg");
    // "../../data/searcher/test/sun_10.jpg"

    ui->textEdit->append("");

    for(uint i=0; i < db_im.size(); i++){
        ui->textEdit->append(QString(db_im[i].fname) + " " + QString::number(db_im[i].dist));
    }
}

void MainWindow::on_listWidget_dbfiles_currentRowChanged(int currentRow)
{
//    ui->textEdit->append(QString::number(currentRow));
    QString path = ui->listWidget_dbfiles->item(currentRow)->text();

    compare_images(path);

    ui->textEdit->clear();
    for(uint i=0; i < db_im.size(); i++){
        ui->textEdit->append(QString(db_im[i].fname) + " " + QString::number(db_im[i].dist));
    }

    /*
     * Упражнение:
     * - сделать вывод гистограмм изображений
    */
}
