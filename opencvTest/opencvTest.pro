TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp
LIBS += -lopencv_core \
#        -lopencv_imgproc \
        -lopencv_imgcodecs \
        -lopencv_highgui
#        -lopencv_objdetect
