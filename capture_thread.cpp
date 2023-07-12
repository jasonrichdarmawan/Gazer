#include "capture_thread.h"
#include "utilities.h"

CaptureThread::CaptureThread(int camera, QMutex *lock) :
    running_(false), cameraID_(camera), videoPath_(""), dataLock_(lock)
{
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock) :
    running_(false), cameraID_(-1), videoPath_(videoPath), dataLock_(lock)
{
}

CaptureThread::~CaptureThread() {
}

// This method is called immediately after the thread is created,
// and when it returns, the lifespan of the thread will finish.
// Therefore, we set the running status to true when we enter this method
// and set the running status to false before returning the method.
void CaptureThread::run() {
    running_ = true;
    cv::VideoCapture cap(cameraID_);
    cv::Mat tmpFrame;
    frameWidth_ = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frameHeight_ = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    while (running_) {
        // we capture a frame and check whether it's empty.
        cap >> tmpFrame;
        if (tmpFrame.empty()) {
            break;
        }
        if (videoSavingStatus_ == STARTING) {
            startSavingVideo(tmpFrame);
        }
        if (videoSavingStatus_ == STARTED) {
            videoWriter_->write(tmpFrame);
        }
        if (videoSavingStatus_ == STOPPING) {
            stopSavingVideo();
        }
        // We are using OpenCV to capture the frames,
        // so the color order of the capture frame is BGR instead of RGB.
        // Considering we will display the frames using Qt,
        // we should convert the frames into new ones with RGB as their color order.
        cv::cvtColor(tmpFrame, tmpFrame, cv::COLOR_BGR2RGB);
        // we use QMutex to ensure that there is only one thread that is accessing the frame member field at any time.
        dataLock_->lock();
        frame_ = tmpFrame;
        dataLock_->unlock();
        emit FrameCaptured(&frame_);
        if (fpsCalculating_) {
            calculateFPS(cap);
        }
    }
    cap.release();
    running_ = false;
}

void CaptureThread::calculateFPS(cv::VideoCapture &cap)
{
    const int countToRead = 100;
    cv::Mat tmpFrame;
    QElapsedTimer timer;
    timer.start();
    for (int i = 0; i < countToRead; i++) {
        cap >> tmpFrame;
    }
    int elapsedMs = timer.elapsed();
    fps_ = countToRead / (elapsedMs / 1000.0);
    fpsCalculating_ = false;
    emit FPSChanged(fps_);
}

void CaptureThread::startSavingVideo(cv::Mat &firstFrame)
{
    savedVideoName_ = Utilities::NewSavedVideoName();

    // we first save the first frame of the video to an image by calling the imwrite function,
    // and this image will be used as the cover of the current video being saved on the UI.
    QString cover = Utilities::GetSavedVideoPath(savedVideoName_, "jpg");
    cv::imwrite(cover.toStdString(), firstFrame);

    videoWriter_ = new cv::VideoWriter(
        Utilities::GetSavedVideoPath(savedVideoName_, "avi").toStdString(),
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
        fps_ ? fps_ : 30,
        cv::Size(frameWidth_, frameHeight_)
    );

    videoSavingStatus_ = STARTED;
}

void CaptureThread::stopSavingVideo()
{
    videoSavingStatus_ = STOPPED;
    videoWriter_->release();
    // deallocate the memory allocated for the `videoWriter_`
    // because videoWriter_ use `new` operator to allocate memory.
    delete videoWriter_;
    videoWriter_ = nullptr;
    emit VideoSaved(savedVideoName_);
}
