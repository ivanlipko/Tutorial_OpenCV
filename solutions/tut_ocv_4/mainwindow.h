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
    void on_pushButton_clicked();

    void on_pushButton_find_clicked();

    void on_listWidget_dbfiles_currentRowChanged(int currentRow);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
