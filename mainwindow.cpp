#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initUI();
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
    imageScene_ = new QGraphicsScene(this);
    imageView_ = new QGraphicsView(imageScene_);
    mainLayout->addWidget(imageView_, 0, 0, 12, 1);

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
    /// we want to align the record push button to the center in the horizontal direction.
    /// To do so, we append a placeholder, a blank QLabel method, to toolsLayout
    toolsLayout->addWidget(new QLabel(this), 0, 2);

    // 14th row
    // list of saved videos.
    savedList_ = new QListView(this);
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
}

void MainWindow::createActions()
{
    cameraInfoAction_ = new QAction("Camera Information");
    openCameraAction_ = new QAction("Open Camera");
    exitAction_ = new QAction("Exit");

    fileMenu_->addAction(cameraInfoAction_);
    fileMenu_->addAction(openCameraAction_);
    fileMenu_->addAction(exitAction_);

    connect(exitAction_, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
}
