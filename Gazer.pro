QT       += core gui multimedia

# Using OpenCV or QCamera
#DEFINES += GAZER_USE_QT_CAMERA=1
#QT += multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Tech Debt: This is a hotfix to ask for Camera permission.
# TODO: how to set bundle identifier
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    capture_thread.cpp \
    main.cpp \
    mainwindow.cpp \
    utilities.cpp

HEADERS += \
    capture_thread.h \
    mainwindow.h \
    utilities.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Add the opencv_imgcodecs OpenCV module to the LIBS setting,
# since the imwrite function we used to save the cover images is offered by that module.
unix: mac {
    INCLUDEPATH += /usr/local/include/opencv4
    LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_video -lopencv_videoio
}
