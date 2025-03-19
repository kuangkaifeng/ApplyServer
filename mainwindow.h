#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QSocketNotifier>
#include <QDebug>
#include <unistd.h>
#include <sys/inotify.h>


#define EVENT_SIZE  (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool startMonitoring(const QStringList &paths);
    void removeAllWatches();  // 清除所有监视
signals:
    void fileEventOccurred(const QString &filePath, const QString &eventType);

private slots:
    void onFileChanged(QString fileName,QString result);
    void handleInotifyEvent();

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    int inotifyFd = -1;  // inotify 文件描述符
    QSocketNotifier *notifier = nullptr;
    QMap<QString, int> watchDescriptors;  // 存储路径和 inotify 监视描述符


};
#endif // MAINWINDOW_H
