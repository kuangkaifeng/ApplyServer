#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QLineEdit>
#include <QProcess>
#include <QComboBox>
#include <QNetworkInterface>
#include <QStringList>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QTimer>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

struct IpPort
{
    QString Ip;
    int port;
};
struct AccountPassword
{
    QString Account;
    QString Password;
};
struct UserInfo
{
    QString Account;
    QString Name;
    QString Sex;
    int Age;
    QString ID;
    QString Address;
    QString Telephone;
    QString Msg;
    QDateTime DateTime;
    QString Status;//审批状态
};

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    //初始化UI界面
    void InitUI(QString title);
    //初始化服务器
    bool InitServer(QString hostAddress,int prot);
    //更新模型
    void updateModel(QListView *listview,QStandardItemModel *model,int flag);
    //获取当前主机的所有IP地址
    QStringList getIpInfo();
    //获取数据库的用户信息
    void getUserInfoFromDatabase();
    //检索数据
    bool IshasUser(QString Account,QString Password);
    //查询有该用户的所有信息
    QJsonObject getDataFromDatabases(QString Account);
    //将申请预约信息加入到数据库
    void ApplyInsertToDatabase(QJsonObject obj);
    QJsonObject Refresh(QString );
private:
    Ui::Widget *ui;

    QFrame *frame1;
    QFrame *frame2;
    QFrame *frame3;
    QVBoxLayout *Fvlayout1;
    QVBoxLayout *Fvlayout2;
    QVBoxLayout *Fvlayout3;
    QListView *listview1;
    QListView *listview2;
    QStandardItemModel *model1;
    QStandardItemModel *model2;
    QLabel *label1;
    QLabel *label2;
    QLabel *label3;
    QLabel *label4;
    QComboBox *comBox_address;
    QLineEdit *lineEdit_prot;
    QVBoxLayout *vlayout;
    QVBoxLayout *vlayout1;
    QHBoxLayout *hlayout;
    QVBoxLayout *layout;
    QHBoxLayout *pagelayout;
    QPushButton *btnDisConnectServer;
    QPushButton *btnStartConnectServer;
    QPushButton *btnDisConnectClient;

    QTcpServer *tcpserver;
    QList <QTcpSocket *> SocketList;
    QStringList selectedItems;
    QStringList ipList;
    //定时器一直获取选中的项
    QTimer *timer;
    //用户信息
    QList<AccountPassword*> AccountInfo;
    QTcpSocket *LastSocket;

private slots:
    //断开服务器
    void disConnectServer();
    //断开客户端连接
    void disConnectClient();
    //开启服务器
    void StartServer();
    //新的客户端连接
    void newConnect();
    //检索选中的项
    void checkSelectedItems();
    //接收客户端信息
    void RecvMsg();

};
#endif // WIDGET_H
