#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    InitUI("服务器");
    //获取本地的账户数据库
    getUserInfoFromDatabase();

    timer=new QTimer(this);
    QObject::connect(timer,&QTimer::timeout,this,&Widget::checkSelectedItems);
    timer->start(100);

}

Widget::~Widget()
{
    delete ui;
    // 释放动态分配的对象
    delete frame1;
    delete frame2;
    delete frame3;

    delete Fvlayout1;
    delete Fvlayout2;
    delete Fvlayout3;

    delete listview1;
    delete listview2;

    delete model1;
    delete model2;

    delete label1;
    delete label2;
    delete label3;
    delete label4;

    delete comBox_address;
    delete lineEdit_prot;

    delete vlayout;
    delete vlayout1;
    delete hlayout;
    delete layout;
    delete pagelayout;

    delete btnDisConnectServer;
    delete btnStartConnectServer;
    delete btnDisConnectClient;

    // 释放 QTcpServer
    delete tcpserver;

    // 释放 SocketList 中的 QTcpSocket 对象
    for (QTcpSocket *socket : SocketList) {
        if (socket) {
            socket->disconnectFromHost(); // 断开连接
            socket->deleteLater();       // 延迟删除
        }
    }
    SocketList.clear(); // 清空列表
}
//
void Widget::InitUI(QString title)
{
    this->setWindowTitle(title);
    this->setWindowIcon(QIcon(":/images/images/server.jpg"));
    frame1=new QFrame();
    frame2=new QFrame();
    frame3=new QFrame();
    //设置为无边框
    frame1->setWindowFlags(Qt::FramelessWindowHint);
    frame2->setWindowFlags(Qt::FramelessWindowHint);
    frame3->setWindowFlags(Qt::FramelessWindowHint);

    listview1=new QListView;
    listview2=new QListView;
    model1=new QStandardItemModel;
    model2=new QStandardItemModel;
    label1=new QLabel("当前连接的客户端");
    label2=new QLabel("选中的客户端");
    label3=new QLabel("ip地址");
    label4=new QLabel("端口号");
    comBox_address=new QComboBox();
    lineEdit_prot=new QLineEdit();
    ipList=getIpInfo();
    comBox_address->addItems(ipList);
    lineEdit_prot->setPlaceholderText("8080");
    comBox_address->setMinimumHeight(40);
    comBox_address->setMaximumHeight(40);
    lineEdit_prot->setMinimumHeight(40);
    lineEdit_prot->setMaximumHeight(40);
    label3->setMinimumHeight(30);
    label3->setMaximumHeight(30);
    label4->setMinimumHeight(30);
    label4->setMaximumHeight(30);
    //设置布局
    Fvlayout1=new QVBoxLayout(frame1);
    Fvlayout2=new QVBoxLayout(frame2);
    Fvlayout3=new QVBoxLayout(frame3);
    Fvlayout3->addWidget(label3);
    Fvlayout3->addWidget(comBox_address);
    Fvlayout3->addWidget(label4);
    Fvlayout3->addWidget(lineEdit_prot);
    Fvlayout3->setSpacing(0);
    Fvlayout3->setContentsMargins(0,0,0,0);
    //设置300的空间
    Fvlayout3->addStretch();
    Fvlayout1->addWidget(label1);
    Fvlayout2->addWidget(label2);
    Fvlayout1->addWidget(listview1);
    Fvlayout2->addWidget(listview2);
    Fvlayout1->setSpacing(0);
    Fvlayout1->setContentsMargins(0,0,0,0);
    Fvlayout2->setSpacing(0);
    Fvlayout2->setContentsMargins(0,0,0,0);
    vlayout=new QVBoxLayout();
    vlayout->addWidget(frame1);
    vlayout->addWidget(frame2);
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0,0,0,0);
    hlayout=new QHBoxLayout();
    layout=new QVBoxLayout();
    btnDisConnectServer=new QPushButton("断开服务器");
    btnStartConnectServer=new QPushButton("开启服务器");
    btnDisConnectClient=new QPushButton("断开客户端");
    hlayout->addStretch();
    hlayout->addWidget(btnDisConnectClient);
    hlayout->addWidget(btnStartConnectServer);
    hlayout->addWidget(btnDisConnectServer);
    hlayout->addStretch();
    hlayout->setSpacing(0);
    hlayout->setContentsMargins(0,0,0,0);
    layout->addLayout(vlayout);
    layout->addLayout(hlayout);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    pagelayout=new QHBoxLayout(this);
    pagelayout->addWidget(frame3);
    pagelayout->addLayout(layout);
    pagelayout->setStretch(0,2);
    pagelayout->setStretch(1,6);
    pagelayout->setSpacing(0);
    pagelayout->setContentsMargins(0,0,0,0);
    //设置模型
    listview1->setModel(model1);
    listview2->setModel(model2);
    listview1->setWindowFlags(Qt::FramelessWindowHint);
    listview2->setWindowFlags(Qt::FramelessWindowHint);
    listview1->setSelectionMode(QAbstractItemView::MultiSelection);
    listview2->setSelectionMode(QAbstractItemView::MultiSelection);

    //关联槽函数
    QObject::connect(btnDisConnectServer,&QPushButton::clicked,this,&Widget::disConnectServer);
    QObject::connect(btnDisConnectClient,&QPushButton::clicked,this,&Widget::disConnectClient);
    QObject::connect(btnStartConnectServer,&QPushButton::clicked,this,&Widget::StartServer);

}
//初始化服务器
bool Widget::InitServer(QString hostAddress, int prot)
{
    tcpserver->setMaxPendingConnections(100);
    QHostAddress address=QHostAddress(hostAddress);
    if (!tcpserver->listen(address, prot)) {
        qDebug() << "监听" << hostAddress << "失败，错误：" << tcpserver->errorString();
        return false;
    }
    qDebug() << "监听" << hostAddress << "成功，端口：" << prot;
    QObject::connect(tcpserver,&QTcpServer::newConnection,this,&Widget::newConnect);
}
//更新模型
void Widget::updateModel(QListView *listview,QStandardItemModel *model,int flag)
{
    model->clear();
    if(flag==1)
    {
        for(int i=0;i<SocketList.size();i++)
        {
            QTcpSocket *socket=SocketList.at(i);
            // 设置 item 的文本（例如显示 socket 的 IP 和端口）
            QString socketInfo = QString("Socket %1:%2:%3")
                                     .arg(i + 1)
                                     .arg(socket->peerAddress().toString())
                                     .arg(socket->peerPort());
            QStandardItem *item=new QStandardItem();
            item->setText(socketInfo);
            model->appendRow(item);
        }
    }
    else if(flag==2)
    {
        for(int i=0;i<selectedItems.size();i++)
        {
            QString buf=selectedItems.at(i);
            QStandardItem *item=new QStandardItem();
            item->setText(buf);
            model->appendRow(item);
        }
    }
    listview->setModel(model);
}
//获取当前主机的所有IP地址
QStringList Widget::getIpInfo()
{
    QStringList ipList;
    // 获取所有网络接口
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &interface : interfaces)
    {
        // 过滤掉未启用或回环接口
        if (!(interface.flags() & QNetworkInterface::IsUp) ||
            interface.flags() & QNetworkInterface::IsLoopBack)
        {
            continue;
        }

        // 获取接口的所有 IP 地址
        QList<QNetworkAddressEntry> entries = interface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries)
        {
            // 只获取 IPv4 地址
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
            {
                ipList.append(entry.ip().toString());
            }
        }
    }

    return ipList;
}
//从数据库获取用户信息加载到
void Widget::getUserInfoFromDatabase()
{
    QString connectionName = "qt_sql_default_connection"; // 默认连接名
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName); // 先移除旧的连接
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
    db.setDatabaseName("kkf");
    db.setUserName("root");
    db.setPassword("123456");

    if (!db.open()) {
        qDebug() << "Error: Failed to connect to database." << db.lastError().text();
        return ;
    }

    QSqlQuery query;

    // 执行 SELECT 查询
    if (query.exec("SELECT * FROM accounts"))
    {
        while (query.next())
        {
            QString Account = query.value(0).toString();  // 获取第一列的值
            QString Password = query.value(1).toString();        // 获取第二列的值
//            qDebug() << "Account:" << Account << "Password:" << Password;
            AccountPassword *userinfo=new AccountPassword;
            userinfo->Account=Account;
            userinfo->Password=Password;
            AccountInfo.append(userinfo);

        }
    }
    else
    {
        qDebug() << "Query failed:" << query.lastError().text();
    }
}
//检索数据库与客户端的用户登录信息
bool Widget::IshasUser(QString Account,QString Password)
{
    for(auto userinfo:AccountInfo)
    {
        if(userinfo->Account==Account&&userinfo->Password==Password)
        {
            return true;
        }
    }
    return false;
}
//获取当前用户信息
QJsonObject Widget::getDataFromDatabases(QString Account)
{
    QJsonObject obj;
    QJsonObject lastObj;

    QString connectionName = "qt_sql_default_connection"; // 默认连接名
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName); // 先移除旧的连接
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
    db.setDatabaseName("kkf");
    db.setUserName("root");
    db.setPassword("123456");

    if (!db.open()) {
        qDebug() << "Error: Failed to connect to database." << db.lastError().text();
        return lastObj;
    }


    //查询用户信息
    QSqlQuery query;
    // 执行 SELECT 查询
    QString cmd=QString("SELECT * FROM userinfo where Account='%1'").arg(Account);
    qDebug()<<"cmd:"<<cmd;

    if (query.exec(cmd))
    {
        while (query.next())
        {
            obj["账户"]=query.value(0).toString();
            obj["姓名"]=query.value(1).toString();
            obj["性别"]=query.value(2).toString();
            obj["年龄"]=query.value(3).toString();
            obj["身份证"]=query.value(4).toString();
            obj["地址"]=query.value(5).toString();
            obj["电话"]=query.value(6).toString();
            obj["审核状态"]=query.value(7).toString();
        }
    }
    else
    {
        qDebug() << "Query failed:" << query.lastError().text();
    }
    //加入个人信息
    lastObj["userinfo"]=obj;
    //查询用户的预约信息
    QSqlQuery query1;
    // 执行 SELECT 查询
    QString cmd1=QString("SELECT * FROM applyinfo where 账户='%1'").arg(Account);
    qDebug()<<"cmd:"<<cmd1;
    QList<QJsonObject> objlist;
    if (query1.exec(cmd1))
    {
        while (query1.next())
        {
            QJsonObject obj1;
            obj1["保密级别"]=query1.value(1).toString();
            obj1["地址"]=query1.value(2).toString();
            obj1["姓名"]=query1.value(3).toString();
            obj1["审批状态"]=query1.value(4).toString();
            obj1["年龄"]=query1.value(5).toString();
            obj1["性别"]=query1.value(6).toString();
            obj1["生成文件密码"]=query1.value(7).toString();
            obj1["电话"]=query1.value(8).toString();

            obj1["账户"]=query1.value(9).toString();
            obj1["身份证"]=query1.value(10).toString();
            obj1["预约名称"]=query1.value(11).toString();
            obj1["预约时间"]=query1.value(12).toString();
            obj1["预约理由"]=query1.value(13).toString();
            obj1["预约甲方"]=query1.value(14).toString();
            obj1["预约类型"]=query1.value(15).toString();
            objlist.append(obj1);
        }
    }
    else
    {
        qDebug() << "Query failed:" << query1.lastError().text();
    }
    // 2. 将 QList<QJsonObject> 转换为 QJsonArray
    QJsonArray jsonArray;
    for (const auto &jsonObj : objlist) {
        jsonArray.append(jsonObj);
    }
    lastObj["applyinfo"] = jsonArray;  // 赋值 QJsonArray

    //查询所有的预约信息
    QSqlQuery query2;
    // 执行 SELECT 查询
    QString cmd2=QString("SELECT * FROM applyinfo");
    qDebug()<<"cmd:"<<cmd2;
    QList<QJsonObject> objlist2;
    if (query2.exec(cmd2))
    {
        while (query2.next())
        {
            QJsonObject obj1;
            obj1["保密级别"]=query2.value(1).toString();
            obj1["地址"]=query2.value(2).toString();
            obj1["姓名"]=query2.value(3).toString();
            obj1["审批状态"]=query2.value(4).toString();
            obj1["年龄"]=query2.value(5).toString();
            obj1["性别"]=query2.value(6).toString();
            obj1["生成文件密码"]=query2.value(7).toString();
            obj1["电话"]=query2.value(8).toString();

            obj1["账户"]=query2.value(9).toString();
            obj1["身份证"]=query2.value(10).toString();
            obj1["预约名称"]=query2.value(11).toString();
            obj1["预约时间"]=query2.value(12).toString();
            obj1["预约理由"]=query2.value(13).toString();
            obj1["预约甲方"]=query2.value(14).toString();
            obj1["预约类型"]=query2.value(15).toString();
            objlist2.append(obj1);
        }
    }
    else
    {
        qDebug() << "Query failed:" << query2.lastError().text();
    }

    QJsonArray jsonArry2;
    for(const auto &jsonObj:objlist2)
    {
        jsonArry2.append(jsonObj);
    }
    lastObj["allapllyinfo"]=jsonArry2;


    return lastObj;
}

void Widget::ApplyInsertToDatabase(QJsonObject obj)
{

    // 处理重复连接
        QString connectionName = "qt_sql_default_connection"; // 默认连接名
        if (QSqlDatabase::contains(connectionName)) {
            QSqlDatabase::removeDatabase(connectionName); // 先移除旧的连接
        }
        QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
        db.setDatabaseName("kkf");
        db.setUserName("root");
        db.setPassword("123456");

        if (!db.open()) {
            qDebug() << "Error: Failed to connect to database." << db.lastError().text();
            return ;
        }


        qDebug() << "数据库连接成功！";

        // 插入数据 SQL 语句
        QString insertSQL = R"(
            INSERT INTO ApplyInfo
            (保密级别, 地址, 姓名, 审批状态, 年龄, 性别, 生成文件密码, 电话, 账户, 身份证, 预约名称, 预约时间, 预约理由, 预约甲方, 预约类型)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        )";

        QSqlQuery query(db);
        query.prepare(insertSQL);
        query.addBindValue(obj["保密级别"].toString());
        query.addBindValue(obj["地址"].toString());
        query.addBindValue(obj["姓名"].toString());
        query.addBindValue(obj["审批状态"].toString());
        query.addBindValue(obj["年龄"].toInt());
        query.addBindValue(obj["性别"].toString());
        query.addBindValue(obj["生成文件密码"].toString());
        query.addBindValue(obj["电话"].toString());
        query.addBindValue(obj["账户"].toString());
        query.addBindValue(obj["身份证"].toString());
        query.addBindValue(obj["预约名称"].toString());

        // 修正预约时间格式
        QString dateTimeStr = obj["预约时间"].toString();
        dateTimeStr.replace("1月", "01").replace("2月", "02").replace("3月", "03")
                   .replace("4月", "04").replace("5月", "05").replace("6月", "06")
                   .replace("7月", "07").replace("8月", "08").replace("9月", "09")
                   .replace("10月", "10").replace("11月", "11").replace("12月", "12");

        QDateTime dateTime = QDateTime::fromString(dateTimeStr, "ddd MM d hh:mm:ss yyyy");

        if (!dateTime.isValid()) {
            qDebug() << "预约时间格式仍然不正确：" << dateTimeStr;
            return;
        }

        query.addBindValue(dateTime.toString("yyyy-MM-dd hh:mm:ss"));
        query.addBindValue(obj["预约理由"].toString());
        query.addBindValue(obj["预约甲方"].toString());
        query.addBindValue(obj["预约类型"].toString());

        // 执行 SQL
        if (!query.exec()) {
            qDebug() << "插入数据失败：" << query.lastError().text();
        } else {
            qDebug() << "数据插入成功！";
        }

        db.close();
}

QJsonObject Widget::Refresh(QString Account)
{
    QJsonObject lastObj;


    QString connectionName = "qt_sql_default_connection"; // 默认连接名
    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName); // 先移除旧的连接
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", connectionName);
    db.setDatabaseName("kkf");
    db.setUserName("root");
    db.setPassword("123456");

    if (!db.open()) {
        qDebug() << "Error: Failed to connect to database." << db.lastError().text();
        return lastObj;
    }



    //查询用户的预约信息
    QSqlQuery query1;
    // 执行 SELECT 查询
    QString cmd1=QString("SELECT * FROM applyinfo where 账户='%1'").arg(Account);
    qDebug()<<"cmd:"<<cmd1;
    QList<QJsonObject> objlist;
    if (query1.exec(cmd1))
    {
        while (query1.next())
        {
            QJsonObject obj1;
            obj1["保密级别"]=query1.value(1).toString();
            obj1["地址"]=query1.value(2).toString();
            obj1["姓名"]=query1.value(3).toString();
            obj1["审批状态"]=query1.value(4).toString();
            obj1["年龄"]=query1.value(5).toString();
            obj1["性别"]=query1.value(6).toString();
            obj1["生成文件密码"]=query1.value(7).toString();
            obj1["电话"]=query1.value(8).toString();

            obj1["账户"]=query1.value(9).toString();
            obj1["身份证"]=query1.value(10).toString();
            obj1["预约名称"]=query1.value(11).toString();
            obj1["预约时间"]=query1.value(12).toString();
            obj1["预约理由"]=query1.value(13).toString();
            obj1["预约甲方"]=query1.value(14).toString();
            obj1["预约类型"]=query1.value(15).toString();
            objlist.append(obj1);
        }
    }
    else
    {
        qDebug() << "Query failed:" << query1.lastError().text();
    }
    // 2. 将 QList<QJsonObject> 转换为 QJsonArray
    QJsonArray jsonArray;
    for (const auto &jsonObj : objlist) {
        jsonArray.append(jsonObj);
    }
    lastObj["applyinfo"] = jsonArray;  // 赋值 QJsonArray

    //查询所有的预约信息
    QSqlQuery query2;
    // 执行 SELECT 查询
    QString cmd2=QString("SELECT * FROM applyinfo");
    qDebug()<<"cmd:"<<cmd2;
    QList<QJsonObject> objlist2;
    if (query2.exec(cmd2))
    {
        while (query2.next())
        {
            QJsonObject obj1;
            obj1["保密级别"]=query2.value(1).toString();
            obj1["地址"]=query2.value(2).toString();
            obj1["姓名"]=query2.value(3).toString();
            obj1["审批状态"]=query2.value(4).toString();
            obj1["年龄"]=query2.value(5).toString();
            obj1["性别"]=query2.value(6).toString();
            obj1["生成文件密码"]=query2.value(7).toString();
            obj1["电话"]=query2.value(8).toString();

            obj1["账户"]=query2.value(9).toString();
            obj1["身份证"]=query2.value(10).toString();
            obj1["预约名称"]=query2.value(11).toString();
            obj1["预约时间"]=query2.value(12).toString();
            obj1["预约理由"]=query2.value(13).toString();
            obj1["预约甲方"]=query2.value(14).toString();
            obj1["预约类型"]=query2.value(15).toString();
            objlist2.append(obj1);
        }
    }
    else
    {
        qDebug() << "Query failed:" << query2.lastError().text();
    }

    QJsonArray jsonArry2;
    for(const auto &jsonObj:objlist2)
    {
        jsonArry2.append(jsonObj);
    }
    lastObj["allapllyinfo"]=jsonArry2;


    return lastObj;
}

//断开服务器
void Widget::disConnectServer()
{
    tcpserver->close();
    qDebug()<<"断开服务器";
}
//断开客户端连接
void Widget::disConnectClient()
{

    for(int i=0;i<model2->rowCount();i++)
    {
        QStandardItem *item=model2->item(i);
        QString buf=item->text();
        QStringList buflist=buf.split(":");
        qDebug()<<"address:"<<buflist.at(1);
        qDebug()<<"prot:"<<buflist.at(2);
        for(int j=0;j<SocketList.size();j++)
        {
            QTcpSocket *socket=SocketList.at(j);
            if(socket->peerAddress().toString()==buflist.at(1)&&socket->peerPort()==buflist.at(2).toUInt())
            {
                socket->disconnectFromHost();
                qDebug()<<"断开"<<socket->peerAddress()<<"客户端连接";
                SocketList.removeOne(socket);
            }
        }
    }
    //更新model1
    updateModel(listview1,model1,1);

}
//开启服务器
void Widget::StartServer()
{
    tcpserver=new QTcpServer();
    //如果服务器处于开启状态
    if(tcpserver->isListening())
    {
        QMessageBox::information(this,"提示","服务器已经开启",QMessageBox::Ok);
        return ;
    }
    if(lineEdit_prot->text().isEmpty())
    {
        lineEdit_prot->setText("8080");
    }
    InitServer(comBox_address->currentText(),lineEdit_prot->text().toInt());
}

void Widget::newConnect()
{
    qDebug()<<"有新的客户端连接";
    QTcpSocket *tcpsocket=tcpserver->nextPendingConnection();
    LastSocket=tcpsocket;
    //登录验证

    SocketList.append(tcpsocket);
    //先更新model1
    updateModel(listview1,model1,1);
    connect(tcpsocket,&QTcpSocket::readyRead,this,&Widget::RecvMsg);
    connect(tcpsocket,&QTcpSocket::disconnected,[=](){
        //移除该客户端
        SocketList.removeOne(tcpsocket);
        qDebug()<<"hostName:"<<tcpsocket->peerName()<<"hostaddress:"<<tcpsocket->peerAddress()<<"prot:"<<tcpsocket->peerPort()<<"断开连接";
        updateModel(listview1,model1,1);
    });
}
//检索选中的项
void Widget::checkSelectedItems()
{
    selectedItems.clear();
    QModelIndexList  selectedIndexs=listview1->selectionModel()->selectedIndexes();
    // 遍历选中的项并获取内容
    for(auto index:selectedIndexs)
    {
        QString text=index.data(Qt::DisplayRole).toString();
        selectedItems.append(text);
//        qDebug()<<"select text:"<<text;
    }
    updateModel(listview2,model2,2);
}

void Widget::RecvMsg()
{
    QByteArray bytes=LastSocket->readAll();
    QJsonDocument doc=QJsonDocument::fromBinaryData(bytes);
    QJsonObject obj=doc.object();
    qDebug()<<"RecvMsg obj:"<<obj;
    //登录操作
    if(obj.find("Login").value()=="true"||obj.find("Reload").value()=="true")
    {
        QJsonObject obj1=obj["Msg"].toObject();
        if(IshasUser(obj1["Account"].toString(),obj1["Password"].toString()))
        {
            //查找有该用户的所有信息（另外一张表）登录成功
            QJsonObject lastObj=getDataFromDatabases(obj1["Account"].toString());

            // 序列化为 JSON 字符串
            QJsonObject rootObj;
            rootObj["sucess"]=true;
            rootObj["data"]=lastObj;
            QJsonDocument doc(rootObj);
            qDebug()<<"doc:"<<doc;
            // 将 QJsonObject 转换为 QByteArray
            QByteArray jsonData = doc.toJson(QJsonDocument::Compact); // 可以选择是否格式化
            LastSocket->write(jsonData);
        }
        else
        {
            //登录失败
            QJsonObject lastObj=getDataFromDatabases(obj1["Account"].toString());

            // 序列化为 JSON 字符串
            QJsonObject rootObj;
            rootObj["sucess"]=false;
            rootObj["data"]=lastObj;
            QJsonDocument doc(rootObj);
            qDebug()<<"doc:"<<doc;
            // 将 QJsonObject 转换为 QByteArray
            QByteArray jsonData = doc.toJson(QJsonDocument::Compact); // 可以选择是否格式化
            LastSocket->write(jsonData);
        }
    }
    //申请预约操作
    else if(obj.find("Apply").value()=="true")
    {
        QJsonObject obj1=obj["Msg"].toObject();
        ApplyInsertToDatabase(obj1);
    }
    else if(obj.find("Refresh").value()=="true")
    {

        //刷新---查询用户的预约信息和所有的预约信息
        QJsonObject lastObj=Refresh(obj.find("Account").value().toString());
        // 序列化为 JSON 字符串
        QJsonObject rootObj;
        rootObj["Refresh"]=true;
        rootObj["data"]=lastObj;
        QJsonDocument doc(rootObj);
        qDebug()<<"doc:"<<doc;
        // 将 QJsonObject 转换为 QByteArray
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact); // 可以选择是否格式化
        LastSocket->write(jsonData);
    }

}

