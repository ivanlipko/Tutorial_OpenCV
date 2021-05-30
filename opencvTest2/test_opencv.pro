#-------------------------------------------------
#
# Project created by QtCreator 2019-04-07T19:36:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_opencv
TEMPLATE = app console


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH += D:\opencv\build\install\include

LIBS += -LD:/opencv/build/install/x86/mingw/bin \
        -lopencv_core401        \
        -lopencv_highgui401     \
        -lopencv_imgcodecs401   \
        -lopencv_imgproc401     \
        -lopencv_features2d401  \
        -lopencv_calib3d401
