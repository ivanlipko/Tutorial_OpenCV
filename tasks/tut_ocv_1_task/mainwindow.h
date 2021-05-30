#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void on_pushButton_readasgray_clicked();

    void on_pushButton_readim_clicked();

    void on_pushButton_chng_bch_clicked();

    void on_pushButton_sum_im_clicked();

    void on_pushButton_load_thr_clicked();

    void on_dial_b_valueChanged(int value);

    void on_dial_g_valueChanged(int value);

    void on_dial_r_valueChanged(int value);

    void on_dial_contrast_valueChanged(int value);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
