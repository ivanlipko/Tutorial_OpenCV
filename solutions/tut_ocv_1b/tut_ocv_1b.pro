#-------------------------------------------------
#
# Project created by QtCreator 2019-05-22T19:36:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tut_ocv_1b
TEMPLATE = app
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui
#
# Ubuntu config section:
#
unix{
    message("UNIX .PRO FILE CONFIG")
    INCLUDEPATH += /usr/local/include/opencv4
    LIBS += -lopencv_core \
    -lopencv_highgui \
    -lopencv_imgcodecs \
    -lopencv_imgproc \
    -lopencv_videoio
} else {

#
# Windows config section:
#
#
    win32{
        message("WINDOWS .PRO FILE CONFIG")
        #INCLUDEPATH += D:\opencv\build\install\include
        INCLUDEPATH += C:\Qt\opencv\build\install\include

        LIBS += -LC:\Qt\opencv\build\install\x86\mingw\bin \
        #LIBS += -LC:\Qt\opencv-4.0.1\build\lib \
        -lopencv_core401 \
        -lopencv_highgui401 \
        -lopencv_imgcodecs401 \
        -lopencv_imgproc401
    }
}
