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
    void nextFrame();

    void on_pushButton_readim_clicked();

    void on_pushButton_cam_clicked();

    void on_pushButton_movieStream_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_clicked();

    void nextFrameFiltered();

    void on_pushButton_runFilterVideo_clicked();

    void on_pushButton_readImtoFilter_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *timer = NULL;
};

#endif // MAINWINDOW_H
