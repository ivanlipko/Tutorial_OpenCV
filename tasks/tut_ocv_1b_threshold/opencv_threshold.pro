TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

INCLUDEPATH += D:\opencv\build\install\include

LIBS += -LD:/opencv/build/install/x86/mingw/bin \
        -lopencv_core401        \
        -lopencv_highgui401     \
        -lopencv_imgcodecs401   \
        -lopencv_imgproc401     \
        -lopencv_features2d401  \
        -lopencv_calib3d401
