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
    Признаки HOG
        https://www.learnopencv.com/handwritten-digits-classification-an-opencv-c-python-tutorial/
        https://www.learnopencv.com/histogram-of-oriented-gradients/

    (!) Алгоритмы обучения
        https://docs.opencv.org/3.4/dc/dd6/ml_intro.html
        https://docs.opencv.org/3.4/d3/d29/ml_8hpp.html

        https://tproger.ru/translations/top-machine-learning-algorithms/
        https://habr.com/ru/post/448892/
        http://www.machinelearning.ru/wiki/index.php?title=Машина_опорных_векторов

    Классификатор kNN (K-Nearest Neighbors) К-ближайших соседей
        https://docs.opencv.org/master/dd/de1/classcv_1_1ml_1_1KNearest.html

    Примеры с кодом:
        https://github.com/opencv/opencv/blob/master/samples/cpp/points_classifier.cpp
        https://github.com/opencv/opencv/blob/master/samples/cpp/letter_recog.cpp

*/

std::vector< cv::Mat > pos_lst, neg_lst, gradient_lst;
std::vector< cv::Mat > test_pos_lst, test_neg_lst, test_gradient_lst;
std::vector< int > labels, test_labels;
cv::Mat train_data, test_data;

//cv::Ptr< cv::ml::SVM > predictor = cv::ml::SVM::create();
cv::Ptr< cv::ml::KNearest > predictor = cv::ml::KNearest::create();
//cv::Ptr< cv::ml::DTrees>
//cv::Ptr< cv::ml::RTrees >
//cv::Ptr< cv::ml::NormalBayesClassifier>
//cv::Ptr< cv::ml::ANN_MLP>


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
size_t load_images( const cv::String & dirname, std::vector< cv::Mat > & img_lst, bool showImages = false )
{
    std::vector< cv::String > files;
    cv::glob( dirname, files );

    for ( size_t i = 0; i < files.size(); ++i )  {
        cv::Mat img = cv::imread( files[i] ); // load the image
        if ( img.empty() ) {            // invalid image, skip it.
            std::cout << files[i] << " is invalid!" << std::endl;
            continue;
        }

        if ( showImages )  {
            cv::imshow( "image", img );
            cv::waitKey( 1 );
        }

        cv::resize(img,img, cv::Size(64,64));

        img_lst.push_back( img );
    }

    return files.size();
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
void convert_to_ml( const std::vector< cv::Mat > & train_samples, cv::Mat& trainData ) {
    //--Convert data
    const int rows = (int)train_samples.size();
    const int cols = (int)std::max( train_samples[0].cols, train_samples[0].rows );
    cv::Mat tmp( 1, cols, CV_32FC1 );  // < used for transposition if needed
    trainData = cv::Mat( rows, cols, CV_32FC1 );

    for( size_t i = 0 ; i < train_samples.size(); ++i )     {
        CV_Assert( train_samples[i].cols == 1 || train_samples[i].rows == 1 );

        if( train_samples[i].cols == 1 )         {
            cv::transpose( train_samples[i], tmp );
            tmp.copyTo( trainData.row( (int)i ) );
        }
        else if( train_samples[i].rows == 1 )         {
            train_samples[i].copyTo( trainData.row( (int)i ) );
        }
    }
}

void MainWindow::on_pushButton_load_faces_clicked()
{
    qDebug() << "Load TRAIN images and calculate HOGs... ";

    gradient_lst.clear();
    pos_lst.clear();
    neg_lst.clear();
    labels.clear();

    test_gradient_lst.clear();
    test_pos_lst.clear();
    test_neg_lst.clear();
    test_labels.clear();

    // Картинки с лицами
    load_images( "../../data/face_recog/1000/faces", pos_lst, false );
    computeHOGs( cv::Size(64,64), pos_lst, gradient_lst, false );
    size_t pos_count = gradient_lst.size();
    labels.assign( pos_count, +1 );

    // Картинки без лиц
    load_images( "../../data/face_recog/1000/backs", neg_lst, false );
    computeHOGs( cv::Size(64,64), neg_lst, gradient_lst, false );
    size_t neg_count = gradient_lst.size() - pos_count;
    labels.insert(labels.end(), neg_count, -1 );

    if ( pos_lst.size() == 0 && neg_lst.size() == 0)    {
        QMessageBox::warning(this, "Error", "Load train data");
        return;
    }

    // Конвертируем в формат, необходимый для алгоритмов-предикторов
    convert_to_ml( gradient_lst, train_data );


    qDebug() << "Load TEST images and calculate HOGs... ";
    test_gradient_lst.clear();
    test_pos_lst.clear();
    test_neg_lst.clear();

    load_images( "../../data/face_recog/test/positive", test_pos_lst, false );
    computeHOGs( cv::Size(64,64), test_pos_lst, test_gradient_lst, false );
    test_labels.assign( test_gradient_lst.size(), +1 );

    load_images( "../../data/face_recog/test/negative", test_neg_lst, false );
    computeHOGs( cv::Size(64,64), test_neg_lst, test_gradient_lst, false );
    test_labels.insert(test_labels.end(), test_neg_lst.size(), -1 );

    if ( test_pos_lst.size() == 0 && test_neg_lst.size() == 0)    {
        QMessageBox::warning(this, "Error", "Load test data");
        return;
    }

    convert_to_ml( test_gradient_lst, test_data );
    qDebug() << "Finished";
}

void MainWindow::on_pushButton_train_clicked()
{
    qDebug() << "Train predictor...";

    /* Default values to train SVM */
    //    predictor->setCoef0( 0.0 );
    //    predictor->setDegree( 3 );
    //    predictor->setGamma( 0 );
    //    predictor->setKernel( cv::ml::SVM::LINEAR );
    //    predictor->setNu( 0.5 );
    //    predictor->setP( 0.1 ); // for EPSILON_SVR, epsilon in loss function?
    //    predictor->setC( 0.01 ); // From paper, soft classifier
    //    predictor->setType( cv::ml::SVM::C_SVC);

    /* kNN */
    //    predictor->setDefaultK(9);
    //    predictor->setIsClassifier(true);

    /*DTree */
    //    predictor->setMaxDepth(15);

    /* Обучаем предиктор */
    predictor->train(train_data, cv::ml::ROW_SAMPLE, labels);
    //        predictor->trainAuto()

    qDebug() << "Finished";

    /*
     * Упражнения:
     * - использовать другие предикторы из исходника https://github.com/opencv/opencv/blob/master/samples/cpp/letter_recog.cpp
     * - прочитать документацию о предикторе и соотнести какой параметр с чем связан
     * (например, ядра-kernel для SVM)
     * - поиграться с параметрами и найти те, которые улучшают качество
     * предсказания
     * - подумать, что это такое и как использовать predictor->trainAuto()
     * - добавьте виджетов для изменения параметров предикторов
    */
}

void MainWindow::on_pushButton_predict_clicked()
{
    cv::Mat pred_responses;
    float err_count;

    if(!predictor->isTrained()){
        QMessageBox::warning(this, "Внимание!", "Надо обучить предиктор" );
        return;
    }

    qDebug() << "Start predict... ";

    predictor->predict(train_data, pred_responses);

    err_count = 0;
    for(int i=0; i < gradient_lst.size(); i++){
        float val = pred_responses.at<float>(i);
        err_count += (float)( val != labels.at(i));
    }
    ui->textEdit_status->append("Доля ошибок на трейне: " + QString::number(err_count/(gradient_lst.size())) );


    predictor->predict(test_data, pred_responses);

    err_count = 0;
    for(int i=0; i < test_gradient_lst.size(); i++){
        float val = pred_responses.at<float>(i);
        err_count += (float)( val != test_labels.at(i));
    }
    ui->textEdit_status->append("Доля ошибок на тесте: " + QString::number(err_count/(test_gradient_lst.size())) );

    qDebug() << "Finish";
}

void MainWindow::on_pushButton_clicked()
{
    //    on_pushButton_load_faces_clicked();
    //    on_pushButton_train_clicked();
    //    on_pushButton_predict_clicked();

    if(!predictor->isTrained()){
        QMessageBox::warning(this, "Внимание!", "Надо обучить предиктор" );
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

            //            predictor->predict(win_data, response, cv::ml::StatModel::RAW_OUTPUT);
            //            float val = response.at<float>(0);
            //            if (val > 2.3) {
            //                ui->textEdit_status->append(QString::number(row) + ", " + QString::number(col) + " = " + QString::number(val) + " / " + QString::number( response.at<double>(0,0) ) );
            //                qDebug() << QString::number(row) <<  QString::number(col) << val << QString::number( response.at<double>(0,0) ) ;
            //                cv::rectangle(test_im, win_rect, cv::Scalar(0,255,0), 1, 8);
            //            }

            predictor->predict(win_data, response);
            float val = response.at<float>(0);
            if (val > 0) {
                ui->textEdit_status->append(QString::number(row) + ", " + QString::number(col) + " = " + QString::number(val) );
                //                qDebug() << QString::number(row) <<  QString::number(col) << val;
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

    /*
     * Упражнения:
     * - реализовать поиск в окне, учитывая масштаб изображений
    */
}

void MainWindow::on_pushButton_save_predictor_clicked()
{
    /*
    cv::Ptr<cv::ml::StatModel> model(predictor);
    model->save("file.xml");
    */
    predictor->save("file.xml");
}

void MainWindow::on_pushButton_load_predictor_clicked()
{
//    predictor = cv::Algorithm::load<cv::ml::KNearest>("file.xml");
}


void MainWindow::on_pushButton_loaddata_clicked()
{
    qDebug() << "Load TRAIN images and calculate HOGs... ";
    gradient_lst.clear();
    pos_lst.clear();
    labels.clear();
    std::vector <int> lb_cnt;  // количество загруженных объектов

    // Картинки со знаками и БЕЗ
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/training/a_unevenness", pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/training/way_out", pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/training/stop", pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/training/road_works", pos_lst, false )  );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/training/pedistrain", pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/training/no_entry", pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/training/no_drive", pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/training/parking", pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/training/none", pos_lst, false ) );

    computeHOGs( cv::Size(64,64), pos_lst, gradient_lst, false );
    // Конвертируем в формат, необходимый для алгоритмов-предикторов
    convert_to_ml( gradient_lst, train_data );

    labels.assign(lb_cnt.front(), +1 ); lb_cnt.erase(lb_cnt.begin());
    labels.insert(labels.end(), lb_cnt.front(), +2 ); lb_cnt.erase(lb_cnt.begin());
    labels.insert(labels.end(), lb_cnt.front(), +3 ); lb_cnt.erase(lb_cnt.begin());
    labels.insert(labels.end(), lb_cnt.front(), +4 ); lb_cnt.erase(lb_cnt.begin());
    labels.insert(labels.end(), lb_cnt.front(), +5 ); lb_cnt.erase(lb_cnt.begin());
    labels.insert(labels.end(), lb_cnt.front(), +6 ); lb_cnt.erase(lb_cnt.begin());
    labels.insert(labels.end(), lb_cnt.front(), +7 ); lb_cnt.erase(lb_cnt.begin());
    labels.insert(labels.end(), lb_cnt.front(), +8 ); lb_cnt.erase(lb_cnt.begin());
    labels.insert(labels.end(), lb_cnt.front(), 0 ); lb_cnt.erase(lb_cnt.begin());


    qDebug() << "Load TEST images and calculate HOGs... ";
    test_gradient_lst.clear();
    test_pos_lst.clear();
    test_labels.clear();

    // Картинки со знаками и БЕЗ
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/val/a_unevenness", test_pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/val/way_out", test_pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/val/stop", test_pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/val/road_works", test_pos_lst, false )  );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/val/pedistrain", test_pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/val/no_entry", test_pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/val/no_drive", test_pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/val/parking", test_pos_lst, false ) );
    lb_cnt.push_back(
                load_images( "../../data/roadsigns/val/none", test_pos_lst, false ) );

    computeHOGs( cv::Size(64,64), test_pos_lst, test_gradient_lst, false );
    convert_to_ml( test_gradient_lst, test_data );

    test_labels.assign(lb_cnt.front(), +1 ); lb_cnt.erase(lb_cnt.begin());
    test_labels.insert(test_labels.end(), lb_cnt.front(), +2 ); lb_cnt.erase(lb_cnt.begin());
    test_labels.insert(test_labels.end(), lb_cnt.front(), +3 ); lb_cnt.erase(lb_cnt.begin());
    test_labels.insert(test_labels.end(), lb_cnt.front(), +4 ); lb_cnt.erase(lb_cnt.begin());
    test_labels.insert(test_labels.end(), lb_cnt.front(), +5 ); lb_cnt.erase(lb_cnt.begin());
    test_labels.insert(test_labels.end(), lb_cnt.front(), +6 ); lb_cnt.erase(lb_cnt.begin());
    test_labels.insert(test_labels.end(), lb_cnt.front(), +7 ); lb_cnt.erase(lb_cnt.begin());
    test_labels.insert(test_labels.end(), lb_cnt.front(), +8 ); lb_cnt.erase(lb_cnt.begin());
    test_labels.insert(test_labels.end(), lb_cnt.front(), 0 ); lb_cnt.erase(lb_cnt.begin());

    qDebug() << "Finished";
}
