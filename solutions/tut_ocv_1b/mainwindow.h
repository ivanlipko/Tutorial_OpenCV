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

    void on_pushButton_run_clicked();

    void on_pushButton_stop_clicked();

    void on_verticalSlider_hsv_low_h_valueChanged(int value);

    void on_verticalSlider_hsv_high_h_valueChanged(int value);

    void on_verticalSlider_hsv_low_s_valueChanged(int value);

    void on_verticalSlider_hsv_high_s_valueChanged(int value);

    void on_verticalSlider_hsv_low_v_valueChanged(int value);

    void on_verticalSlider_hsv_high_v_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    QTimer *timer = NULL;

    void setGBColor();
};

#endif // MAINWINDOW_H
