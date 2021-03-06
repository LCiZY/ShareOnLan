#include "shareonlan.h"
#include "ui_shareonlan.h"


ShareOnLan::ShareOnLan(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ShareOnLan)
{
    ui->setupUi(this);
    Log("程序开始运行");

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
    connect(server,SIGNAL(ipChange()),this,SLOT(ipChange()));
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
    Log("程序结束运行");
}

void ShareOnLan::windowInit(){

    ui->right_top_icon_label->setPixmap(QPixmap(":/icon/transfer_3d.png"));
    ui->lineEdit_port->setText(conf->getConfig("port"));valid_port=new QIntValidator(this); valid_port->setRange(1025,65533);ui->lineEdit_port->setValidator(valid_port); ui->lineEdit_port->setToolTip("端口号范围：1025 - 65534");
    ui->checkBox_ifhidewhenlaunch->setChecked(conf->getConfig("ifhidewhenlaunch").compare("true")==0);
    ui->checkBox_ifautostartup->setChecked(conf->getConfig("automaticStartup").compare("true")==0); conf->setAutomaticStartup(ui->checkBox_ifautostartup->isChecked());
    ui->lineEdit_secret->setText(conf->getConfig("secret")); ui->lineEdit_secret->setToolTip("输入手机APP上的密钥");
    ui->lineEdit_fileReceiveLocation->setText(conf->getConfig("fileReceiveLocation"));
    ui->lineEdit_fileReceiveLocation->setToolTip(ui->lineEdit_fileReceiveLocation->text());
    ui->lineEdit_fileReceiveLocation->setEnabled(false);
    ui->pushButton_changeFileReceiveLocation->setToolTip("更改文件保存位置"); ui->pushButton_changeFileReceiveLocation->setIcon(QIcon(":/images/button_edit_location.png")); ui->pushButton_changeFileReceiveLocation->setIconSize(QSize(21,21));
    ui->pushButton_openFileLocationFolder->setToolTip("打开保存文件的文件夹");ui->pushButton_openFileLocationFolder->setIcon(QIcon(":/images/button_directory.png")); ui->pushButton_openFileLocationFolder->setIconSize(QSize(21,21));

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
    case QSystemTrayIcon::Unknown:
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::MiddleClick:
        break;
    case QSystemTrayIcon::Context:
        //右击托盘图标
        QPainterPath path;
        QRectF rect = QRectF(0,0,mMenu->sizeHint().width(),mMenu->sizeHint().height());
        path.addRoundedRect(rect,4,4);
        QPolygon polygon= path.toFillPolygon().toPolygon();
        QRegion region(polygon);    mMenu->setMask(region);
        break;

    }
}


void ShareOnLan::sysTrayMenuInit(){

        mMenu = new QMenu(this);
        // 设置圆角的前置设置
//        mMenu->setWindowFlags(Qt::FramelessWindowHint);
//        mMenu->setAttribute(Qt::WA_TranslucentBackground);

        mSendToPhoneAction = new QAction(QIcon(":/images/icon_message.png"),tr("发送文本"),mMenu);
        connect(mSendToPhoneAction,SIGNAL(triggered()),this,SLOT(on_SendToPhone()));

        mSendFile = new QAction(QIcon(":/images/icon_file.png"),tr("发送文件"),mMenu);
        connect(mSendFile,SIGNAL(triggered()),this,SLOT(on_SendFile()));

        mConnectInfoAction = new QAction(QIcon(":/images/icon_ip.png"),tr("本机IP"),mMenu);
        connect(mConnectInfoAction,SIGNAL(triggered()),this,SLOT(on_ShowNetInfo()));

        mRestartServiceAction = new QAction(QIcon(":/images/icon_reboot.png"),tr("重启服务"),mMenu);
        connect(mRestartServiceAction,SIGNAL(triggered()),this,SLOT(on_restartServer()));

        mSettingAction = new QAction(QIcon(":/images/icon_setting.png"),tr("设置"),mMenu);
        connect(mSettingAction,SIGNAL(triggered()),this,SLOT(on_showSettingAction()));


        mExitAppAction = new QAction(tr("退出"),mMenu);
        connect(mExitAppAction,SIGNAL(triggered()),this,SLOT(on_exitAppAction()));




            //新增菜单项---发送文本
        mMenu->addAction(mSendToPhoneAction);
           //新增菜单项---发送文件
        mMenu->addAction(mSendFile);
           //增加分隔符----------
        mMenu->addSeparator();
           //新增菜单项---关闭连接
        mMenu->addAction(mConnectInfoAction);
           //新增菜单项---重启服务
        mMenu->addAction(mRestartServiceAction);
           //新增菜单项---显示主界面
        mMenu->addAction(mSettingAction);
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

void ShareOnLan::ipChange(){
    server->getLanBrocastAddress();
    sysTrayTextChange();
}

void ShareOnLan::sysTrayTextChange(){
    QString content="";
    for(int i=0;i<ipList.size();i++) {
        content.append("\n").append("IP:").append(ipList.at(i)).append(" (").append(networkcardList.at(i)).append(")");
    }
    tray_tooltip = tr("「ShareOnLan」正在运行").append("\n连接状态:").append(server->getConnection()).append(content).append("\n端口:").append(QString::number(server->listeningPort));
    //当鼠标移动到托盘上的图标时，会显示此处设置的内容
    mSysTrayIcon->setToolTip(tray_tooltip);
}



void ShareOnLan::on_SendToPhone(){
    if(!this->server->ifConnected()) { QMessageBox::about(nullptr,"失败","未连接至手机");return;}
    //写入剪贴板
    QClipboard *board = QApplication::clipboard();

    this->server->sendMsg(board->text()+"\n");
}

void ShareOnLan::on_SendFile(){
    if(!this->server->ifConnected()) { QMessageBox::about(nullptr,"失败","未连接至手机");return;}
    this->fileserver->ifSend = true;
    //用户选择要发送的文件，可能会阻塞很久
    QString userChooseFileName = QFileDialog::getOpenFileName(nullptr,"请选择要发送的文件",QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    this->server->sendMsg("${FILEINFO}"+getFileInfo(userChooseFileName));
    this->fileserver->setSendFileName(userChooseFileName);
}

void ShareOnLan::on_showSettingAction(){
    this->showMaximized();
    this->move(QApplication::desktop()->availableGeometry().width()-rect().width()/2,QApplication::desktop()->availableGeometry().height()-rect().height()/2);
}

//显示本机IP等信息
void ShareOnLan::on_ShowNetInfo(){
    QMessageBox::about(nullptr,"连接信息",tray_tooltip);
}

//断开所有连接,清空文件接收队列
void ShareOnLan::clearConnection(){
    server->closeSocket();
    fileserver->closeSocket();
    //清空文件接收队列
    receiveFilesNameQueue.clear();
    receiveFilesSizeQueue.clear();
}

//重启server：重新监听端口并resume连接并重新检测网络信息
void ShareOnLan::on_restartServer(){
    clearConnection();
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
    conf->setConfig("automaticStartup",flag?"true":"false");
    conf->setAutomaticStartup(flag);
}

void ShareOnLan::showMini(){
    this->showMinimized();
}

void ShareOnLan::hideWindow(){
    this->hide();
}

void ShareOnLan::on_exitAppAction(){
    this->server->close();
    this->fileserver->close();
    qApp->exit();
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
    /**以下四句为正常关闭的代码*/
//     this->server->close();
//     this->fileserver->close();
//     qApp->exit();
//     event->accept();
       this->hide();
       event->ignore();
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

