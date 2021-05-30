#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QMessageBox>

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
    Классификатор SVM
        https://docs.opencv.org/master/d1/d73/tutorial_introduction_to_svm.html
        https://docs.opencv.org/master/d0/df8/samples_2cpp_2train_HOG_8cpp-example.html#a48
        https://github.com/opencv/opencv/blob/master/modules/ml/src/svm.cpp
        возвращаемый результат  predict
        http://www.compvision.ru/forum/index.php?/topic/2080-svmpredict-%D0%B2%D0%BE%D0%B7%D0%B2%D1%80%D0%B0%D1%89%D0%B0%D0%B5%D1%82-int-%D0%B2%D0%BC%D0%B5%D1%81%D1%82%D0%BE-float/
    Ядра, параметры
        https://docs.opencv.org/master/d1/d2d/classcv_1_1ml_1_1SVM.html#aad7f1aaccced3c33bb256640910a0e56
    Признаки HOG
        https://www.learnopencv.com/handwritten-digits-classification-an-opencv-c-python-tutorial/
        https://www.learnopencv.com/histogram-of-oriented-gradients/
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
 * -----------------------------------------------------------------------------
*/
void load_images( const cv::String & dirname, std::vector< cv::Mat > & img_lst, bool showImages = false )
{
    std::vector< cv::String > files;
    cv::glob( dirname, files );
    for ( size_t i = 0; i < files.size(); ++i )
    {
        cv::Mat img = cv::imread( files[i] ); // load the image
        if ( img.empty() )            // invalid image, skip it.
        {
            std::cout << files[i] << " is invalid!" << std::endl;
            continue;
        }
        if ( showImages )
        {
            cv::imshow( "image", img );
            cv::waitKey( 1 );
        }
        cv::resize(img,img, cv::Size(64,64));
        img_lst.push_back( img );
    }
}

void computeHOGs( const cv::Size wsize, const std::vector< cv::Mat > & img_lst, std::vector< cv::Mat > & gradient_lst, bool use_flip )
{
    cv::HOGDescriptor hog;
    hog.winSize = wsize;
    cv::Mat gray;
    std::vector< float > descriptors;
    for( size_t i = 0 ; i < img_lst.size(); i++ ) {
        if ( img_lst[i].cols >= wsize.width && img_lst[i].rows >= wsize.height ) {
            cv::Rect r = cv::Rect(( img_lst[i].cols - wsize.width ) / 2,
                                  ( img_lst[i].rows - wsize.height ) / 2,
                                  wsize.width,
                                  wsize.height);
            cv::cvtColor( img_lst[i](r), gray, cv::COLOR_BGR2GRAY );
            hog.compute( gray, descriptors, cv::Size( 8, 8 ), cv::Size( 0, 0 ) );
            //            hog.compute( gray, descriptors, cv::Size( 8, 8 ));
            gradient_lst.push_back( cv::Mat( descriptors ).clone() );
            if ( use_flip ) {
                cv::flip( gray, gray, 1 );
                hog.compute( gray, descriptors, cv::Size( 8, 8 ), cv::Size( 0, 0 ) );
                gradient_lst.push_back( cv::Mat( descriptors ).clone() );
            }
        }
    }
    /*
     * Упражнение:
     * - поиграться с параметрами у hog:
     *  winSize(64,128), blockSize(16,16), blockStride(8,8),
        cellSize(8,8), nbins(9)
     * - как влияет на качество прогноза?
     * - как влияет на скорость вычислений?
    */
}

/*
* Convert training/testing set to be used by OpenCV Machine Learning algorithms.
* TrainData is a matrix of size (#samples x max(#cols,#rows) per samples), in 32FC1.
* Transposition of samples are made if needed.
*/
void convert_to_ml( const std::vector< cv::Mat > & train_samples, cv::Mat& trainData )
{
    //--Convert data
    const int rows = (int)train_samples.size();
    const int cols = (int)std::max( train_samples[0].cols, train_samples[0].rows );
    cv::Mat tmp( 1, cols, CV_32FC1 ); //< used for transposition if needed
    trainData = cv::Mat( rows, cols, CV_32FC1 );
    for( size_t i = 0 ; i < train_samples.size(); ++i )
    {
        CV_Assert( train_samples[i].cols == 1 || train_samples[i].rows == 1 );
        if( train_samples[i].cols == 1 )
        {
            cv::transpose( train_samples[i], tmp );
            tmp.copyTo( trainData.row( (int)i ) );
        }
        else if( train_samples[i].rows == 1 )
        {
            train_samples[i].copyTo( trainData.row( (int)i ) );
        }
    }
}

/*
 * -----------------------------------------------------------------------------
*/


std::vector< cv::Mat > pos_lst, neg_lst, gradient_lst;
std::vector< int > labels, test_labesls;
cv::Mat train_data, test_data;

void MainWindow::on_pushButton_rand_face_clicked()
{
    gradient_lst.clear();
    pos_lst.clear();
    neg_lst.clear();

    ui->pushButton_rand_face->setVisible(false);
    ui->textEdit_status->append("Loading and caluclate HOG... ");

    load_images( "../../data/face_recog/2000/faces", pos_lst, false );
    computeHOGs( cv::Size(64,64), pos_lst, gradient_lst, false );
    size_t positive_count = gradient_lst.size();
    labels.assign( positive_count, +1 );

    load_images( "../../data/face_recog/2000/backs", neg_lst, false );
    computeHOGs( cv::Size(64,64), neg_lst, gradient_lst, false );
    size_t negative_count = gradient_lst.size() - positive_count;
    labels.insert(labels.end(), negative_count, -1 );

    if ( pos_lst.size() == 0 && neg_lst.size() == 0)    {
        QMessageBox::warning(this, "Error", "Load data");
        ui->textEdit_status->append("Loading ERROR");
        return;
    }

    convert_to_ml( gradient_lst, train_data );

    ui->pushButton_rand_face->setVisible(true);
    ui->textEdit_status->append("OK");
}


cv::Ptr< cv::ml::SVM > svm = cv::ml::SVM::create();

void MainWindow::on_pushButton_train_clicked()
{
    ui->textEdit_status->append("Start train ...");
    qDebug() << "Start train ...";

    /* Default values to train SVM */
    svm->setCoef0( 0.0 );
    svm->setDegree( 3 );
    //    svm->setTermCriteria( TermCriteria(TermCriteria::MAX_ITER + TermCriteria::EPS, 1000, 1e-3 ) );
    svm->setGamma( 0 );
    svm->setKernel( cv::ml::SVM::LINEAR );
    svm->setNu( 0.5 );
    svm->setP( 0.1 ); // for EPSILON_SVR, epsilon in loss function?
    svm->setC( 0.01 ); // From paper, soft classifier
    svm->setType( cv::ml::SVM::C_SVC);

    svm->train(train_data, cv::ml::ROW_SAMPLE, labels);
    //    svm->trainAuto()
    ui->textEdit_status->append("Finished");
    qDebug() << "Finished";

    /*
     * Упражнение:
     * - прочитать документацию про SVM и соотнести какой параметр связан с чем
     * в формулах с ядрами (kernel)
     * - поиграться с параметрами и найти те, которые улучшают качество
     * предсказания
    */
}

void MainWindow::on_pushButton_oredict_clicked()
{
    gradient_lst.clear();
    pos_lst.clear();
    neg_lst.clear();

    ui->textEdit_status->append("Load test samples ... ");
    qDebug() << "Load test samples ... ";

    load_images( "../../data/face_recog/test/positive", pos_lst, false );
    computeHOGs( cv::Size(64,64), pos_lst, gradient_lst, false );
    size_t positive_count = gradient_lst.size();
    test_labesls.assign( positive_count, +1 );

    load_images( "../../data/face_recog/test/negative", neg_lst, false );
    computeHOGs( cv::Size(64,64), neg_lst, gradient_lst, false );
    size_t negative_count = neg_lst.size();
    test_labesls.insert(test_labesls.end(), negative_count, -1 );

    if ( pos_lst.size() == 0 && neg_lst.size() == 0)    {
        QMessageBox::warning(this, "Error", "Load data");
        ui->textEdit_status->append("Loading ERROR");
        return;
    }

    convert_to_ml( gradient_lst, test_data );
    ui->textEdit_status->append("Ok");
    qDebug() << "Ok";

    cv::Mat pred_test_responses;
    float error;
    qDebug() << "isTrained == " << svm->isTrained();

    if(svm->isTrained()){
        ui->textEdit_status->append("Start predict");
        qDebug() << "Start predict";

        svm->predict(test_data, pred_test_responses);

        for(int i=0; i < negative_count + positive_count; i++){
            float val = pred_test_responses.at<float>(i);
            error += (float)( val != test_labesls.at(i));
        }
        ui->textEdit_status->append("Доля ошибок " + QString::number(error/(positive_count+negative_count)) );

    } else{
        QMessageBox::warning(this, "Внимание!", "Надо обучить класификатор" );
        ui->textEdit_status->append("Надо обучить класификатор");
        return;
    }
    ui->textEdit_status->append("Finish");
    qDebug() << "Finish";
}

void MainWindow::on_pushButton_clicked()
{
    on_pushButton_rand_face_clicked();
    on_pushButton_train_clicked();

    if(!svm->isTrained()){
        ui->textEdit_status->append("ERROR. SVM must be trained!");
        qDebug() << "ERROR. SVM must be trained!";
        return;
    }

    cv::Mat test_im, grid_im;
    test_im = cv::imread("../../data/face_recog/families/5.jpg");
    cv::imshow("Original", test_im);
    grid_im = test_im.clone();

    // Окно, HOG (и сконвертированное) и предсказание
    std::vector< cv::Mat > win_im, win_hog;
    cv::Mat win_data, response;

//       ----- - - - - - удалить//     Размер окна должен быть такой же как для HOGa
    int win_rows =128;
    int win_cols = 128;
    int stepSlide = 8;

    for (int row = 0; row <= test_im.rows - win_rows; row += stepSlide) {
        for (int col = 0; col <= test_im.cols - win_cols; col += stepSlide) {
            cv::Rect win_rect(col, row, win_rows, win_cols);

//            grid_im = test_im.clone(); //
//            cv::rectangle(grid_im, win_rect, cv::Scalar(0,255,0), 1, 8);

            cv::Mat temp = test_im(win_rect).clone();
            cv::resize(temp, temp, cv::Size(64,64));
            win_im.push_back( temp );

            computeHOGs( cv::Size(64,64), win_im, win_hog, false );
            convert_to_ml( win_hog, win_data );
            svm->predict(win_data, response, cv::ml::StatModel::RAW_OUTPUT);
            float val = response.at<float>(0);

            if (val > 2.3) {
                ui->textEdit_status->append(QString::number(row) + ", " + QString::number(col) + " = " + QString::number(val) + " / " + QString::number( response.at<double>(0,0) ) );
                qDebug() << QString::number(row) <<  QString::number(col) << val << QString::number( response.at<double>(0,0) ) ;
                cv::rectangle(test_im, win_rect, cv::Scalar(0,255,0), 1, 8);
            }

//            cv::imshow("grid_im", grid_im);
//            cv::waitKey(150);

            // очищаем чтобы в списке была одна картинка
            win_im.clear();
            win_hog.clear();
        }
    }

    cv::imshow("grid_im", test_im);

}
