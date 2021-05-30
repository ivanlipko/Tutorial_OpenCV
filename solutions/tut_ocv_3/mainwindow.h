#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_pushButton_load_faces_clicked();

    void on_pushButton_train_clicked();

    void on_pushButton_predict_clicked();

    void on_pushButton_clicked();

    void on_pushButton_save_predictor_clicked();

    void on_pushButton_load_predictor_clicked();

    void on_pushButton_loaddata_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
