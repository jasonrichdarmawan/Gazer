#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QCheckBox>
#include <QPushButton>
#include <QListView>
#include <QStatusBar>
#include <QLabel>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initUI();
    void createActions();

private:
    QMenu *fileMenu_;

    // actions on menu bar
    QAction *cameraInfoAction_;
    QAction *openCameraAction_;
    QAction *exitAction_;

    // video area
    QGraphicsScene *imageScene_;
    QGraphicsView *imageView_;

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
};
#endif // MAINWINDOW_H
