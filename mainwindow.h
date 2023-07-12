#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>

#ifdef GAZER_USE_QT_CAMERA
#include <QCameraViewfinder>
#include <QCamera>
#else
#include <QGraphicsScene>
#include <QGraphicsView>
#endif

#include <QCheckBox>
#include <QPushButton>
#include <QListView>
#include <QStatusBar>
#include <QLabel>

#include <QStandardItemModel>

#include "capture_thread.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initUI();
    void createActions();
    void populateSavedList();

private:
    QMenu *fileMenu_;

    // actions on menu bar
    QAction *cameraInfoAction_;
    QAction *openCameraAction_;
    QAction *exitAction_;

    QAction *calcFPSAction_;

    // video area
#ifdef GAZER_USE_QT_CAMERA
    QCamera *camera_;
    QCameraViewfinder *viewfinder_;
#else
    QGraphicsScene *imageScene_;
    QGraphicsView *imageView_;
#endif

    // actions on the operation toolbar
    /// tell us whether the security monitor status is turned on or not.
    /// if it's checked, our application will perform motion detection
    /// and send a notification when something happens.
    /// otherwise, the application will only work as a camera player.
    QCheckBox *monitorCheckBox_;
    /// used to start or stop recording a video.
    QPushButton *recordButton_;

    // the horizontal list for saved videos.
    QListView *savedList_;

    // status bar
    QStatusBar *mainStatusBar_;
    QLabel *mainStatusLabel_;

    cv::Mat currentFrame_;

    // for capture thread
    // The QMutext object, data_lock, is used to protect the data of CaptureThread.frame in the race conditions.
    QMutex *dataLock_;
    CaptureThread *capturer_;

    QStandardItemModel *listModel_;

private slots:
    void showCameraInfo();
    void openCamera();
    void updateFrame(cv::Mat*);
    void calculateFPS();
    void updateFPS(float);
    void recordingStartStop();
    void appendSavedVideo(QString name);
};
#endif // MAINWINDOW_H
