#include "shareonlan.h"
#include "ui_shareonlan.h"


ShareOnLan::ShareOnLan(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ShareOnLan)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("ShareOnLan"));
    this->setWindowIcon(QIcon(":/icon/transfer_64.png"));
    this->setWindowFlag(Qt::FramelessWindowHint,true);
    this->setAttribute(Qt::WA_TranslucentBackground,true);

    QApplication::setQuitOnLastWindowClosed(false);

    server = new msgServer(this,conf->getConfig("port").toInt());
    fileserver = new fileServer;

    windowInit();
    sysTrayIconInit();
    sysTrayMenuInit();

    varInit();

    connect(server,SIGNAL(clientChange()),this,SLOT(clientChange()));
    connect(fileserver,SIGNAL(newFileConnection()),this,SLOT(showProgressUI()));
    connect(fileserver,SIGNAL(fileTransferDone()),this,SLOT(progressUIDestroy()));
    connect(fileserver,SIGNAL(currFileInfo(int,QString)),this,SLOT(setProgressInfo(int,QString)));
    connect(fileserver,SIGNAL(receiveProgress(qint64)),this,SLOT(progressUIChange(qint64)));
    connect(fileserver,SIGNAL(sendProgress(qint64)),this,SLOT(progressUIChange(qint64)));


}

ShareOnLan::~ShareOnLan()
{
    Log("析构调用");
    conf->deleteLater();
    fileserver->deleteLater();
    delete ui;
    Log("析构调用完成");
}

void ShareOnLan::windowInit(){

    ui->lineEdit_port->setText(conf->getConfig("port"));valid_port=new QIntValidator(this); valid_port->setRange(1025,65533);ui->lineEdit_port->setValidator(valid_port); ui->lineEdit_port->setToolTip("端口号范围：1025 - 65534");
    ui->checkBox_ifhidewhenlaunch->setChecked(conf->getConfig("ifhidewhenlaunch").compare("true")==0);
    ui->checkBox_ifautostartup->setChecked(conf->getConfig("ifautoStartup").compare("true")==0);
    ui->lineEdit_secret->setText(conf->getConfig("secret")); ui->lineEdit_secret->setToolTip("输入手机APP上的密钥");
    ui->lineEdit_fileReceiveLocation->setText(conf->getConfig("fileReceiveLocation"));
    ui->lineEdit_fileReceiveLocation->setToolTip(ui->lineEdit_fileReceiveLocation->text());
    ui->lineEdit_fileReceiveLocation->setEnabled(false);
    ui->pushButton_changeFileReceiveLocation->setToolTip("更改文件保存位置");
    ui->pushButton_openFileLocationFolder->setToolTip("打开保存文件的文件夹");

    QFile styleFile(":/style/style.qss");if(styleFile.open(QIODevice::ReadOnly)){QString style=styleFile.readAll(); this->setStyleSheet(style);}

    connect(ui->lineEdit_secret,SIGNAL(textChanged(QString)),this,SLOT(secretChange(QString)));
    connect(ui->lineEdit_port,SIGNAL(textChanged(QString)),this,SLOT(portChange(QString)));
    connect(ui->checkBox_ifhidewhenlaunch,SIGNAL(toggled(bool)),this,SLOT(slot_checkBox_ifhidewhenlaunch(bool)));
    connect(ui->checkBox_ifautostartup,SIGNAL(toggled(bool)),this,SLOT(slot_checkBox_autoLaunch(bool)));
    connect(ui->pushButton_changeFileReceiveLocation,SIGNAL(clicked(bool)),this,SLOT(changeFileReceiveLocation()));
    connect(ui->pushButton_openFileLocationFolder,SIGNAL(clicked(bool)),this,SLOT(openFileLocationFolder()));
    connect(ui->minimalButton,SIGNAL(clicked(bool)),this,SLOT(showMini()));
    connect(ui->closeButton,SIGNAL(clicked(bool)),this,SLOT(hideWindow()));


}


void ShareOnLan::varInit(){
    //拖动窗口移动标志
    moveFlag=false;

    progressui = nullptr;


}

void ShareOnLan::setWinFlags(){
    //设置窗口本身是否可见
    this->setVisible(!ui->checkBox_ifhidewhenlaunch->isChecked());
    //设置托盘图标
    this->mSysTrayIcon->setVisible(true);
}

void ShareOnLan::sysTrayIconInit(){

        //创建QSystemTrayIcon对象
        mSysTrayIcon = new QSystemTrayIcon(this);
        //设置QSystemTrayIcon图标
        mSysTrayIcon->setIcon(QIcon(":/icon/transfer.png"));
        //为托盘图标绑定槽函数
        connect(mSysTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason)));
}

void ShareOnLan::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason){
    case QSystemTrayIcon::DoubleClick:
        //双击托盘图标
        //双击后显示主程序窗口
        this->showMaximized();
        this->move(QApplication::desktop()->availableGeometry().width()-rect().width()/2,QApplication::desktop()->availableGeometry().height()-rect().height()/2);
        break;
    default:
        break;
    }
}


void ShareOnLan::sysTrayMenuInit(){

        mMenu = new QMenu(this);

        mSendToPhoneAction = new QAction(tr("发送文本"),mMenu);
        connect(mSendToPhoneAction,SIGNAL(triggered()),this,SLOT(on_SendToPhone()));

        mSendFile = new QAction(tr("发送文件"),mMenu);
        connect(mSendFile,SIGNAL(triggered()),this,SLOT(on_SendFile()));

        mClearConnectionAction = new QAction(tr("关闭连接"),mMenu);
        connect(mClearConnectionAction,SIGNAL(triggered()),this,SLOT(on_clearConnection()));

        mRestartServiceAction = new QAction(tr("重启服务"),mMenu);
        connect(mRestartServiceAction,SIGNAL(triggered()),this,SLOT(on_restartServer()));

        mShowMainAction = new QAction(tr("显示主界面"),mMenu);
        connect(mShowMainAction,SIGNAL(triggered()),this,SLOT(on_showMainAction()));


        mExitAppAction = new QAction(tr("退出"),mMenu);
        connect(mExitAppAction,SIGNAL(triggered()),this,SLOT(on_exitAppAction()));




            //新增菜单项---发送文本
        mMenu->addAction(mSendToPhoneAction);
           //新增菜单项---发送文件
        mMenu->addAction(mSendFile);
           //增加分隔符----------
        mMenu->addSeparator();
           //新增菜单项---关闭连接
        mMenu->addAction(mClearConnectionAction);
           //新增菜单项---重启服务
        mMenu->addAction(mRestartServiceAction);
           //新增菜单项---显示主界面
        mMenu->addAction(mShowMainAction);
           //增加分隔符----------
        mMenu->addSeparator();
           //新增菜单项---退出程序
        mMenu->addAction(mExitAppAction);
           //把QMenu赋给QSystemTrayIcon对象
        mSysTrayIcon->setContextMenu(mMenu);

           //初始化tooltip
        sysTrayTextChange();
}



//监听的端口号有变更
void  ShareOnLan::portChange(QString newPort){
    if(newPort.isEmpty()||newPort.isNull()||newPort.compare("")==0||newPort.toInt()<=1024) return;
    conf->setConfig("port",newPort);
    server->listenOn(newPort.toInt());
    sysTrayTextChange();
}

void ShareOnLan::secretChange(QString newSecret){
    if(newSecret.isNull()||newSecret.compare("")==0) return;
    newSecret = newSecret.simplified(); newSecret = newSecret.replace(" ","");
    conf->setConfig("secret",newSecret);
}

void  ShareOnLan::slot_checkBox_ifhidewhenlaunch(bool checked){
    conf->setConfig("ifhidewhenlaunch",checked?"true":"false");
}


void ShareOnLan::changeFileReceiveLocation(){
    QString currDir = ui->lineEdit_fileReceiveLocation->text().compare("")==0?QStandardPaths::writableLocation(QStandardPaths::DesktopLocation):ui->lineEdit_fileReceiveLocation->text();
    QString location = QFileDialog::getExistingDirectory(this,tr("选择文件保存位置"),currDir);
    if(location.compare("")==0) return;
    ui->lineEdit_fileReceiveLocation->setText(location);
    ui->lineEdit_fileReceiveLocation->setToolTip(location);
    conf->setConfig("fileReceiveLocation",location);

}

void ShareOnLan::openFileLocationFolder(){
      QString currDir = ui->lineEdit_fileReceiveLocation->text().compare("")==0?QStandardPaths::writableLocation(QStandardPaths::DesktopLocation):ui->lineEdit_fileReceiveLocation->text();
      if(currDir.compare("")==0) return;
      QDesktopServices::openUrl(QUrl(tr("file:///")+currDir,QUrl::TolerantMode));
}



void ShareOnLan::clientChange(){
    //托盘tooltip改变
    sysTrayTextChange();
    //如果控制连接断开：进度条页面存在则关闭； 关闭filesocket
    if(!server->ifConnected()) { if(this->progressui!=nullptr) progressUIDestroy(); fileserver->closeSocket();  }
}

void ShareOnLan::sysTrayTextChange(){
    QString ipports="";
    foreach (QString ip, ipList) {
        ipports.append("\n").append("IP:").append(ip).append("  端口:").append(QString::number(server->listeningPort));
    }
    //当鼠标移动到托盘上的图标时，会显示此处设置的内容
    mSysTrayIcon->setToolTip(tr("「ShareOnLan」正在运行")+
                             ipports+
                             tr("\n连接状态：")+QString(server->getConnection()));
}



void ShareOnLan::on_SendToPhone(){
    //写入剪贴板
    QClipboard *board = QApplication::clipboard();

    this->server->sendMsg(board->text()+"\n");
}

void ShareOnLan::on_SendFile(){
    if(!this->server->ifConnected()) { QMessageBox::about(nullptr,"错误","连接失败");return;}
    this->fileserver->ifSend = true;
    //用户选择要发送的文件，可能会阻塞很久
    QString userChooseFileName = QFileDialog::getOpenFileName(nullptr,"请选择要发送的文件",QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    this->server->sendMsg("${FILEINFO}"+getFileInfo(userChooseFileName));
    this->fileserver->setSendFileName(userChooseFileName);
}

void ShareOnLan::on_showMainAction(){
    this->showMaximized();
    this->move(QApplication::desktop()->availableGeometry().width()-rect().width()/2,QApplication::desktop()->availableGeometry().height()-rect().height()/2);
}


//断开所有连接,清空文件接收队列
void ShareOnLan::on_clearConnection(){
    server->closeSocket();
    fileserver->closeSocket();
    //清空文件接收队列
    receiveFilesNameQueue.clear();
    receiveFilesSizeQueue.clear();
}

//重启server：重新监听端口并resume连接并重新检测网络信息
void ShareOnLan::on_restartServer(){
    on_clearConnection();
    server->getLanBrocastAddress();
    server->serverShutDown();
    fileserver->serverShutDown();
    server->listenOn(server->listeningPort);
    server->resumeAccepting();
    fileserver->listenOn(fileserver->fileServerListeningPort);
    fileserver->resumeAccepting();
    sysTrayTextChange();
}

void ShareOnLan::slot_checkBox_autoLaunch(bool flag){
    conf->setConfig("ifautoStartup",flag?"true":"false");
    conf->setAutomaticStartup(flag);
}

void ShareOnLan::showMini(){
    this->showMinimized();
}

void ShareOnLan::hideWindow(){
    this->hide();
}

void ShareOnLan::on_exitAppAction(){
    this->close();
}

void  ShareOnLan::showProgressUI(){
    if(this->progressui!=nullptr) { progressui->show(); return;}
     progressui = new progressUI(nullptr);
     connect(progressui,SIGNAL(destroyed(QObject*)),this,SLOT(progressUIDestroy()));
     progressui->showAtBottomRight();

}


void  ShareOnLan::setProgressInfo(int fileSize, QString fileName){
    if(this->progressui==nullptr) return;
    progressui->setCurrTaskInfo(fileSize,fileName);
}

void ShareOnLan::progressUIChange(qint64 pg){
    if(this->progressui==nullptr) return;
    this->progressui->changeUI(pg);
}

void ShareOnLan::progressUIDestroy(){
    if(this->progressui==nullptr) return;
    this->progressui->close();
    this->progressui->deleteLater();
    this->progressui=nullptr;
}































void ShareOnLan::mouseMoveEvent(QMouseEvent *event)
{
    if(moveFlag)
    {
     endPoint= event->globalPos() - startPoint + w_startPoint;
     this->move(endPoint);

    }

}

void ShareOnLan::mousePressEvent(QMouseEvent *event){

    if(event->globalX()>this->frameGeometry().topLeft().x() && event->globalX()<this->frameGeometry().topRight().x() && event->globalY()>this->frameGeometry().topLeft().y() && event->globalY()<this->frameGeometry().topLeft().y()+30)
   {
         startPoint = event->globalPos();
         w_startPoint= this->frameGeometry().topLeft();
         moveFlag=true;
    }

}

void ShareOnLan::mouseReleaseEvent(QMouseEvent *)
{
    moveFlag=false;

}

//改写了关闭事件，先关闭服务器监听循环再接受关闭事件
void ShareOnLan::closeEvent(QCloseEvent* event){

     this->server->close();
     this->fileserver->close();
     qApp->exit();
     event->accept();

}







bool ShareOnLan::detectSingleInstance(){

    QString serverName = QCoreApplication::applicationName();
        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(500)) { //如果能够连接得上的话，将参数发送到服务器，然后退出
            Log("程序已经运行");
            return true;
        }
    //运行到这里，说明没有实例在运行，那么创建服务器。
        m_localServer = new QLocalServer(this);
        connect(m_localServer, SIGNAL(newConnection()),
                this, SLOT(newLocalSocketConnection())); //监听新到来的连接

        if (!m_localServer->listen(serverName)) {
            if (m_localServer->serverError() == QAbstractSocket::AddressInUseError
                && QFile::exists(m_localServer->serverName())) { //确保能够监听成功
                QFile::remove(m_localServer->serverName());
                m_localServer->listen(serverName);
            }

        }
    return false;
}

void ShareOnLan::newLocalSocketConnection(){

    QLocalSocket *socket = m_localServer->nextPendingConnection();

        if (!socket) return;
        socket->waitForReadyRead(1000);
        QTextStream stream(socket);
        socket->deleteLater();
        this->show();
        this->raise();
        this->activateWindow();

}

