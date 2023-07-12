#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGridLayout>
#include <QCameraInfo>
#include <QMessageBox>
#include <QDIr>

#include "utilities.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
    , capturer_(nullptr)
{
    initUI();
    dataLock_ = new QMutex();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    this->resize(1000,800);
    // set up the menu bar.
    fileMenu_ = menuBar()->addMenu("File");

    QGridLayout *mainLayout = new QGridLayout();
#ifdef GAZER_USE_QT_CAMERA
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    camera_ = new QCamera(cameras[0]);
    viewfinder_ = new QCameraViewfinder(this);
    QCameraViewfinderSettings settings;
    // the size must be compatible with the camera
    settings.setResolution(QSize(800,600));
    camera_->setViewfinder(viewfinder_);
    camera_->setViewfinderSettings(settings);
    mainLayout->addWidget(viewfinder_, 0, 0, 12, 1);
#else
    imageScene_ = new QGraphicsScene(this);
    imageView_ = new QGraphicsView(imageScene_);
    mainLayout->addWidget(imageView_, 0, 0, 12, 1);
#endif

    // 13th row
    QGridLayout *toolsLayout = new QGridLayout();
    mainLayout->addLayout(toolsLayout, 12, 0, 1, 1);

    // 13th row, 1st column
    monitorCheckBox_ = new QCheckBox(this);
    monitorCheckBox_->setText("Monitor On/Off");
    toolsLayout->addWidget(monitorCheckBox_, 0, 0);

    recordButton_ = new QPushButton(this);
    recordButton_->setText("Record");
    // 13th row, 2nd column
    toolsLayout->addWidget(recordButton_, 0, 1, Qt::AlignHCenter);
    connect(recordButton_, SIGNAL(clicked(bool)), this, SLOT(recordingStartStop()));
    /// we want to align the record push button to the center in the horizontal direction.
    /// To do so, we append a placeholder, a blank QLabel method, to toolsLayout
    toolsLayout->addWidget(new QLabel(this), 0, 2);

    // 14th row
    // list of saved videos.
    // The QListView class is designed to follow the model/view pattern.
    // In this pattern, the model that holds the data is separated from the view, which is in charge of representing the data.
    // Therefore, we need a model to provide the data for it.
    savedList_ = new QListView(this);

    // to ensure that its items will be laid out using the LeftToRight flow with a large size.
    savedList_->setViewMode(QListView::IconMode);

    // to ensure that its items will be laid out every time the view is resized.
    savedList_->setResizeMode(QListView::Adjust);

    // to ensure there are proper spaces between the items and that all the items will be placed in one row,
    // no matter how many of them there are.
    savedList_->setSpacing(5);
    savedList_->setWrapping(false);

    listModel_ = new QStandardItemModel(this);
    savedList_->setModel(listModel_);
    mainLayout->addWidget(savedList_, 13, 0, 4, 1);

    // we can't directly call this->setLayout(mainLayout);
    // because the main window has its own way to manage its content.
    // we can create a new widget that will have the main grid layout as its layout
    // and then set this widget as the central widget of the main window.
    QWidget *widget = new QWidget();
    widget->setLayout(mainLayout);
    setCentralWidget(widget);

    // set up the status bar
    mainStatusBar_ = statusBar();
    mainStatusLabel_ = new QLabel(mainStatusBar_);
    mainStatusBar_->addPermanentWidget(mainStatusLabel_);
    mainStatusLabel_->setText("Gazer is Ready");

    createActions();
    populateSavedList();
}

void MainWindow::createActions()
{
    cameraInfoAction_ = new QAction("Camera Information");
    openCameraAction_ = new QAction("Open Camera");
    exitAction_ = new QAction("Exit");
    calcFPSAction_ = new QAction("Calculate FPS");

    fileMenu_->addAction(cameraInfoAction_);
    fileMenu_->addAction(openCameraAction_);
    fileMenu_->addAction(exitAction_);
    fileMenu_->addAction(calcFPSAction_);

    connect(cameraInfoAction_, SIGNAL(triggered(bool)), this, SLOT(showCameraInfo()));
    connect(openCameraAction_, SIGNAL(triggered(bool)), this, SLOT(openCamera()));
    connect(exitAction_, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(calcFPSAction_, SIGNAL(triggered(bool)), this, SLOT(calculateFPS()));
}

void MainWindow::showCameraInfo() {
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QString info = QString("Available Cameras: \n");

    foreach (const QCameraInfo &cameraInfo, cameras) {
        info += " - " + cameraInfo.deviceName() + ": ";
        info += cameraInfo.description() + "\n";
    }

    QMessageBox::information(this, "Cameras", info);
}

#ifdef GAZER_USE_QT_CAMERA
// This version is simple—it sets the capturing mode of the camera, and then calls the start method of the camera.
// No explicit things about threading need to be handled since the QCamera class will handle it for us.”
void MainWindow::openCamera()
{
    camera_->setCaptureMode(QCamera::CaptureVideo);
    camera_->start();
}
#else
void MainWindow::openCamera()
{
    if (capturer_ != nullptr) {
        // if a thread is already running, stop it
        capturer_->SetRunning(false);
        disconnect(capturer_, &CaptureThread::FrameCaptured, this, &MainWindow::updateFrame);
        // After the infinite loop ends and the run method returns,
        // the thread will go to the end of its lifetime,
        // and its finished signal will be emitted.
        // Because of the connection from the finished signal to the deleteLater slot,
        // the deleteLater slot will be called once the thread ends.
        // As a result, the Qt library will delete this thread instance
        // when the control flow of the program returns to the event loop of the Qt library.
        connect(capturer_, &CaptureThread::finished, capturer_, &CaptureThread::deleteLater);

        disconnect(capturer_, &CaptureThread::FPSChanged, this, &MainWindow::updateFPS);

        disconnect(capturer_, &CaptureThread::VideoSaved, this, &MainWindow::appendSavedVideo);
    }

    int camID = 0;
    capturer_ = new CaptureThread(camID, dataLock_);
    connect(capturer_, &CaptureThread::FrameCaptured, this, &MainWindow::updateFrame);
    connect(capturer_, &CaptureThread::FPSChanged, this, &MainWindow::updateFPS);
    connect(capturer_, &CaptureThread::VideoSaved, this, &MainWindow::appendSavedVideo);
    capturer_->start();
    // Then, we connect the frameCaptured signal of capturer to the updateFrame slot of our main window,
    // so that when the CaptureThread::frameCaptured signal is emitted,
    // the MainWindow::updateFrame slot (method) will be called with the same argument
    // that's used when the signal is emitted.
    mainStatusLabel_->setText(QString("Capturing Camera %1").arg(camID));
}
#endif

void MainWindow::updateFrame(cv::Mat *mat)
{
    dataLock_->lock();
    currentFrame_ = *mat;
    dataLock_->unlock();

    QImage frame(
        currentFrame_.data,
        currentFrame_.cols,
        currentFrame_.rows,
        currentFrame_.step,
        QImage::Format_RGB888);
    QPixmap image = QPixmap::fromImage(frame);

#ifndef GAZER_USE_QT_CAMERA
    imageScene_->clear();
    imageView_->resetTransform();
    imageScene_->addPixmap(image);
    imageScene_->update();
    imageView_->setSceneRect(image.rect());
#endif
}

void MainWindow::calculateFPS()
{
    if (capturer_ != nullptr) {
        capturer_->StartCalcFPS();
    }
}

void MainWindow::updateFPS(float fps)
{
    mainStatusLabel_->setText(QString("FPS of current camera is %1").arg(fps));
}

// The recordingStartStop slot is for the recordButton push button.
void MainWindow::recordingStartStop()
{
    QString text = recordButton_->text();
    if (text == "Record" && capturer_ != nullptr) {
        capturer_->SetVideoSavingStatus(CaptureThread::STARTING);
        recordButton_->setText("Stop Recording");
    } else if (text == "Stop Recording" && capturer_ != nullptr) {
        capturer_->SetVideoSavingStatus(CaptureThread::STOPPING);
        recordButton_->setText("Record");
    }
}

// we emit a signal when a video file is completely saved—the CaptureThread::videoSaved signal.
// The MainWindow::appendSavedVideo slot is for this signal.
void MainWindow::appendSavedVideo(QString name)
{
    QString cover = Utilities::GetSavedVideoPath(name, "jpg");

    // A QStandarditem item is an item with a standard icon image and a string.
    // Its icon is too small for our UI design, so we use an empty item as a placeholder,
    // and then set a big image as the decoration data in its position later.
    QStandardItem *item = new QStandardItem();
    listModel_->appendRow(item);
    QModelIndex index = listModel_->indexFromItem(item);

    // to set a QPixmap object, which is constructed from the cover image and scaled to its proper size
    // in the position that's indicated by the found index for the Qt::DecorationRole role.
    listModel_->setData(index, QPixmap(cover).scaledToHeight(145), Qt::DecorationRole);

    // we set the video name as the display data for the Qt::DisplayRole role in the same position.
    listModel_->setData(index, name, Qt::DisplayRole);

    savedList_->scrollTo(index);
}

void MainWindow::populateSavedList()
{
    QDir dir(Utilities::GetDataPath());
    QStringList nameFilters;
    nameFilters << "*.jpg";
    QFileInfoList files = dir.entryInfoList(nameFilters, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);

    foreach(QFileInfo cover, files) {
        QString name = cover.baseName();
        QStandardItem *item = new QStandardItem();
        listModel_->appendRow(item);
        QModelIndex index = listModel_->indexFromItem(item);
        listModel_->setData(index, QPixmap(cover.absoluteFilePath()).scaledToHeight(145), Qt::DecorationRole);
        listModel_->setData(index, name, Qt::DisplayRole);
    }
}
