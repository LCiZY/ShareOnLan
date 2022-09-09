#include "msgserver.h"

msgServer::msgServer(QObject *, int port)
{
    this->listeningPort = port;
    this->socket = nullptr;
    this->udpSocket = new QUdpSocket;
    this->msg = "";


    getLanBrocastAddress();
    checkClientAliveTimerID = startTimer((timeOutValue-4)*1000); // timeOutValue是Android端读超时时间readTimeOutValue，PC端必须在超时之前发送心跳包给Android端
    checkIPClientTimerID = startTimer(4000);
}

msgServer::~msgServer(){
    socket->deleteLater();
}


void msgServer::incomingConnection(int descriptor){

    if(this->socket!=nullptr){ log::warn("%s","已存在其他TCP连接！拒绝连接");  return; }
    pauseAccepting();
    QTcpSocket* socket = new QTcpSocket(this);
    socket->setSocketDescriptor(descriptor);
    incomingConnection(socket);

}

void msgServer::incomingConnection(QTcpSocket* socket){
    if(this->socket!=nullptr){ this->socketDisconnect(); }

    this->socket = socket;
    socket->setSocketOption(QAbstractSocket::KeepAliveOption,1); //设置keepalive连接
    if(!this->socket->waitForConnected()) {log::info("%s",tr("error:%1").arg(this->socket->errorString()).toStdString().c_str());}
    connect(socket,SIGNAL(readyRead()),this,SLOT(readMsg()));
    connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnect()));
    log::info("%s", (tr("接收到控制连接, socketDescriptor: ")+QString::number(socket->socketDescriptor())+tr("  ip:")+getIPv4( socket->peerAddress().toIPv4Address())+tr("  port:")+QString::number(socket->peerPort())).toStdString().c_str());
    //发送client改变消息
    emit(clientChange());

}


void msgServer::socketDisconnect(){

    if(this->socket!=nullptr){
        log::info("%s",(tr("控制连接断开，socket descriptor：")+QString::number(this->socket->socketDescriptor())).toStdString().c_str());
        disconnect(socket,SIGNAL(readyRead()),this,SLOT(readMsg()));
        disconnect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnect()));
        this->socket->deleteLater();
        this->socket = nullptr;
        //清空文件接收队列
        AppContext::receiveFilesQueue.clear();
        AppContext::sendFilesQueue.clear();
        //恢复监听
        resumeAccepting();
        //发送client改变消息
        emit(clientChange());
    }
}


void msgServer::readMsg(){

    if(this->socket != nullptr)
        while (this->socket->bytesAvailable()>0) {
            int length = this->socket->bytesAvailable();
            char* buf = new char[length+1];
            this->socket->read(buf,length); buf[length]='\0';
            lastMsg =QString::fromUtf8(buf);


            if(lastMsg.indexOf(AppContext::FileInfoMsgPreffix)==0){//收到文件信息
                log::info("%s", (tr("接收到移动端文件信息：")+lastMsg).toStdString().c_str());
                FileInfo* rt = utils::parseFileInfoMsg(lastMsg);
                AppContext::receiveFilesQueue.enqueue(rt);
                send(FILEINFORESPONSE);
            }else if(lastMsg.compare(FILEINFORESPONSE)==0){//对方已经收到文件信息了
                emit otherPCReadyReceiveFile();
            }else if(lastMsg.compare(RESPONSE)==0){//心跳包
                if(this->msgHeartStack.size()) this->msgHeartStack.pop();
            }else{//文本信息，写入剪贴板
                log::info("%s",(tr("接收到移动端文本消息：")+lastMsg).toStdString().c_str());
                QClipboard *board = QApplication::clipboard();
                board->setText(lastMsg);msgList.append(lastMsg);
                //自动回复: R\n
                send(RESPONSE);
            }

        }

}

QMutex mutex;

void msgServer::send(const char* msg){
    mutex.lock();
    if(this->socket!=nullptr){
        this->socket->write(msg, strlen(msg));
    }
    mutex.unlock();
}

void msgServer::sendMsg(QString msg){
    mutex.lock();
    if(this->socket!=nullptr){
        //将换行符用其他字符替代
        msg = msg.replace("\r",REPLACER);
        msg = msg.replace("\n",REPLACEN);
        msg = msg + "\n";
        std::string temp = msg.toStdString();
        this->socket->write(temp.c_str(),strlen(temp.c_str()));
    }
    mutex.unlock();
}

//断开socket连接，关闭socket
void msgServer::closeSocket(){
    if(this->socket!=nullptr)this->socket->close();
}

//服务停止，停止监听端口
void msgServer::serverShutDown(){
    if(this->socket!=nullptr)this->socket->close();
    this->close();
}

//服务启动，监听某个端口
bool msgServer::listenOn(int port){
    close();
    bool ok;
    if((ok = listen(QHostAddress::Any,port))){
        log::info("%s",QString("消息服务器：监听成功，端口：%1").arg(QString::number(port)).toStdString().c_str());
        this->listeningPort = port;
    }else{
        log::error("%s",QString("消息服务器：监听失败，端口：%1").arg(QString::number(port)).toStdString().c_str());
    }
    return ok;
}


bool msgServer::ifConnected(){
    return this->socket!=nullptr;
}

QString msgServer::getConnection(){
    return ifConnected()?("连接至"+getIPv4(this->socket->peerAddress().toIPv4Address())):"无连接";
}

const QTcpSocket* msgServer::getSocket(){
    return this->socket;
}

int errorTimes = 0;
void msgServer::timerEvent(QTimerEvent *e){

    /**检查连接是否正常*/
    if(checkClientAliveTimerID == e->timerId()){
        //如果已经连接，则heartbeat：每 2 秒自动回复: R\n
        if(this->socket!=nullptr){
            //  qDebug("State:%d",this->socket->state()); //3是正常的状态：connected
            if(this->socket->state()!=3) {errorTimes++; if(errorTimes==2){this->socket->close();errorTimes=0; socketDisconnect(); } log::error("检测到连接异常(wrong state)，自动断开"); }
            send(RESPONSE);
            this->socket->waitForBytesWritten();
            msgHeartStack.push(0); if(msgHeartStack.size()>2){this->socket->close();errorTimes=0;msgHeartStack.clear();socketDisconnect(); log::error("检测到连接异常(timeout)，自动断开"); }
        }else if(AppContext::ipList.size()!=0){
            //如果还没连接：发送ip、端口号、连接密钥的信息
            for(int i=0;i<AppContext::ipList.size();i++){
                netInfoStr = AppContext::ipList.at(i)+tr(" ")+setting->get(UserSetting::Item::PORT)+tr(" ");
                QHostAddress host  = QHostAddress(AppContext::brocastList.at(i));
                QString info = encrypt(netInfoStr+setting->get(UserSetting::Item::SECRET));
                std::string strTemp= info.toStdString();
                const char* temp = strTemp.c_str();
                //qDebug()<<"发送UDP报文："<<netInfoStr+setting->get(UserSetting::Item::SECRET)+"  加密后："<<temp;
                this->udpSocket->writeDatagram(temp,strlen(temp), host, DEFAULT_IP_PORT_UDP_PORT);
                this->udpSocket->waitForBytesWritten();
            }
        }
    }

    /**检查网卡信息是否变化*/
    if(checkIPClientTimerID == e->timerId()) {
        getCurrIPList();
        bool networkChange = false;
        if(AppContext::ipListCurr.size()!=AppContext::ipList.size()){
            networkChange = true;
        } else {
            bool flag = true;
            for(int i=0;i<AppContext::ipListCurr.size();i++) {
                bool flag_1 = false;
                for(int j=0;j<AppContext::ipList.size();j++){
                    if(AppContext::ipListCurr.at(i)==AppContext::ipList.at(j)) {flag_1 = true; break;}
                }
                flag = flag & flag_1;
            }
            networkChange = !flag;
        }
        /**发送信号使UI更新托盘信息*/
        if(networkChange) {
            log::info("%s","网卡IP改变，更新托盘ToolTip");
            emit(ipChange());
        }
    }

}

QString msgServer::encrypt(QString msg){

    srand(QTime::currentTime().msec());
    char c1 = ((char)(33+rand()%33));
    srand(QTime::currentTime().second()*QTime::currentTime().minute());
    char c2 = ((char)(33+rand()%33));
    char mc = (c1+c2)/2;
    int factor[3] = {8,2,7};
    std::string temp = msg.toStdString();
    int j;
    for(j=temp.length()-1;temp[j]!=' ';j--){}
    for(int i=0;i<=j;i++){
        temp[i] = (char)((int)temp[i]+pow(-1,i)*factor[i%3]+(i*mc)/temp.length());
    }
    for(uint i=j+1;i<temp.length();i++){
        temp[i] = (char)((int)temp[i]-2*i/3);
    }
    QString result = QString(temp.c_str());
    result.prepend(QChar(c1)); result.append(QChar(c2));
    return result;
}

QString msgServer::getIPv4(qint32 ip){
    QString result = "";
    int count=0;
    while (count<3) {
        result =(":"+QString::number(ip&0xFF)) + result;
        ip = ip>>8;
        count++;
    }
    result=QString::number(ip&0xFF) + result;
    return result;
}

// 拿到当前网卡的ip等信息，提供给托盘tooltip
void  msgServer::getLanBrocastAddress(){
    AppContext::ipList.clear(); AppContext::brocastList.clear();  AppContext::networkcardList.clear();
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    log::info("%s","获取网卡信息：");
    for (int i = 0; i < interfaceList.count(); i++)
    {
        QNetworkInterface interf = interfaceList.at(i);

        QNetworkInterface::InterfaceFlags interFlags;
        interFlags = interf.flags();
        if((interFlags&QNetworkInterface::IsUp) &&
                (interFlags&QNetworkInterface::IsRunning) &&
                (interFlags&QNetworkInterface::CanBroadcast) &&
                (interFlags&QNetworkInterface::CanMulticast) &&
                !(interFlags&QNetworkInterface::IsLoopBack)){ }else continue;

        QString networkCardName=interf.humanReadableName();
        if(networkCardName.contains("Loopback")) continue;

        // qDebug()<<"网卡名称："<<networkCardName; //接口名称（网卡）
        log::info("%s", (tr("网卡名称：")+networkCardName).toStdString().c_str());


        // 读取一个IP地址列表
        QList<QNetworkAddressEntry> entryList = interf.addressEntries();
        for(int j = 0; j < entryList.count(); j++)
        {

            QNetworkAddressEntry entry = entryList.at(j);

            QString ip = entry.ip().toString();                             //IP地址
            QString mask = entry.netmask().toString();            //子网掩码
            QString bcast = entry.broadcast().toString();          //广播地址
            if(bcast.compare("")==0) continue;

            AppContext::ipList.append(ip);
            AppContext::brocastList.append(bcast);
            AppContext::networkcardList.append(networkCardName);

            log::info("%s", (tr("ip：")+ip+tr("  掩码：")+mask+"  广播地址："+bcast).toStdString().c_str());

        }

    }
}

// 由定时器触发，拿到当前的ip列表
void  msgServer::getCurrIPList(){
    AppContext::ipListCurr.clear();
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    for (int i = 0; i < interfaceList.count(); i++)
    {
        QNetworkInterface interf = interfaceList.at(i);
        QNetworkInterface::InterfaceFlags interFlags;
        interFlags = interf.flags();
        if((interFlags&QNetworkInterface::IsUp) &&
                (interFlags&QNetworkInterface::IsRunning) &&
                (interFlags&QNetworkInterface::CanBroadcast) &&
                (interFlags&QNetworkInterface::CanMulticast) &&
                !(interFlags&QNetworkInterface::IsLoopBack)){ }else continue;

        QString networkCardName=interf.humanReadableName();
        if(networkCardName.contains("Loopback")) continue;

        QList<QNetworkAddressEntry> entryList = interf.addressEntries();
        for(int j = 0; j < entryList.count(); j++)
        {
            QNetworkAddressEntry entry = entryList.at(j);
            QString ip = entry.ip().toString();                             //IP地址
            QString bcast = entry.broadcast().toString();          //广播地址
            if(bcast.compare("")==0) continue;
            AppContext::ipListCurr.append(ip);
        }

    }


}

