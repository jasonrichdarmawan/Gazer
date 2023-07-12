#ifndef CAPTURE_THREAD_H
#define CAPTURE_THREAD_H

#include <QString>
#include <QThread>
#include <QMutex>

#include "opencv2/opencv.hpp"

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    enum VideoSavingStatus {
        STARTING,
        STARTED,
        STOPPING,
        STOPPED
    };

public:
    // The first constructor accepts an integer, which is the index of the target webcam,
    // and a pointer of QMutex, which will be used to protect data in a race condition.
    CaptureThread(int camera, QMutex *lock);
    // The second constructor accepts a string that will be treated as a path to a video file and a QMutex pinter.
    // With this constructor, we can use a video file to emulate a webcam.
    CaptureThread(QString videoPath, QMutex *lock);
    ~CaptureThread();

public:
    // There's also a public method called SetRunning,
    // which is used to set the running status of the capturing thread
    void SetRunning(bool run) { running_ = run; };

    // to trigger an FPS calculation.
    void StartCalcFPS() { fpsCalculating_ = true; };

    void SetVideoSavingStatus(VideoSavingStatus status) { videoSavingStatus_ = status; };

protected:
    // The override keyword indicates that this method is a virtual method
    // and that it is overriding a method with the same name as one of its base class' methods.
    // The run method of QThread is the starting point for a thread.
    // When we call the start method of a thread, its run method will be called after the new thread is created.
    // We will do the capturing work in this method later.
    void run() override;

signals:
    // This signal will be emitted each time a frame is captured from the webcam.
    // If you are intersted in this signal, you can connect a slot to it.
    void FrameCaptured(cv::Mat *data);
    void FPSChanged(float fps);

    void VideoSaved(QString name);

private:
    void calculateFPS(cv::VideoCapture &cap);

    void startSavingVideo(cv::Mat &firstName);
    void stopSavingVideo();

private:
    bool running_; // for the thread state
    int cameraID_; // for the camera index
    QString videoPath_; // for the path of the video that is used to emulate a webcam

    QMutex *dataLock_; // for protecting data in race conditions
    cv::Mat frame_; // for storing the currently captured frame.

    // FPS calculating
    bool fpsCalculating_ = false; // to indicate whether the capturing thread is doing, or should do, an FPS calculation or not.
    float fps_ = 0.0; // to save the calculated FPS.

    // video saving
    int frameWidth_, frameHeight_ = 0;
    VideoSavingStatus videoSavingStatus_ = STOPPED;
    QString savedVideoName_ = "";
    cv::VideoWriter *videoWriter_ = nullptr;
};

#endif // CAPTURE_THREAD_H
