#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{

}
bool MainWindow::startMonitoring(const QStringList &paths)
{
    if (inotifyFd != -1) {
        qWarning() << "Monitor is already running!";

        return false;
    }

    inotifyFd = inotify_init();
    if (inotifyFd < 0) {
        qWarning() << "Failed to initialize inotify!";
        LOG_DEBUG("Failed to initialize inotify!");
        return false;
    }

    for (const QString &path : paths) {
        int wd = inotify_add_watch(inotifyFd, path.toUtf8().constData(),
                                   IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
        if (wd < 0) {
            qWarning() << "Failed to add watch for path:" << path;
            LOG_DEBUG(QString("Failed to add watch for path:") + path);
        } else {
            watchDescriptors[path] = wd;
            LOG_DEBUG(QString("Started monitoring: ") + path);
        }
    }

    if (watchDescriptors.isEmpty()) {
        close(inotifyFd);
        inotifyFd = -1;
        return false;
    }

    notifier = new QSocketNotifier(inotifyFd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &FileMonitor_uos::handleInotifyEvent);

    return true;
}
