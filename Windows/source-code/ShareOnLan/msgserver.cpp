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

    if(this->socket!=nullptr){ Log(tr("已存在其他TCP连接！拒绝连接"));  return; }
    pauseAccepting();
    this->socket = new QTcpSocket(this);
    this->socket->setSocketDescriptor(descriptor);
    this->socket->setSocketOption(QAbstractSocket::KeepAliveOption,1); //设置keepalive连接
    if(!this->socket->waitForConnected()) {Log(tr("error:%1").arg(this->socket->errorString()));}
    connect(socket,SIGNAL(readyRead()),this,SLOT(readMsg()));
    connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnect()));
    Log(tr("接收到控制连接：")+QString::number(descriptor)+tr("  ip:")+getIPv4( socket->peerAddress().toIPv4Address())+tr("  port:")+QString::number(socket->peerPort()));
    //发送client改变消息
    emit(clientChange());
}

void msgServer::socketDisconnect(){

    if(this->socket!=nullptr){
        Log(tr("控制连接断开：")+QString::number(this->socket->socketDescriptor()));
        disconnect(socket,SIGNAL(readyRead()),this,SLOT(readMsg()));
        disconnect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnect()));
        this->socket->deleteLater();
        this->socket = nullptr;
        //清空文件接收队列
        receiveFilesQueue.clear();
        sendFilesQueue.clear();
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


        if(lastMsg.indexOf(FILE_INFO_MSG_HEAD)==0){//如果是文件信息
           Log(tr("接收到移动端文件信息：")+lastMsg);
           FileInfo* rt = parseFileInfoMsg(lastMsg);
           receiveFilesQueue.enqueue(rt);
           //自动回复: FILEINFO R\n
           this->socket->write(FILEINFORESPONSE,strlen(FILEINFORESPONSE));
        }else if(lastMsg.compare(RESPONSE)==0){
           if(this->msgHeartStack.size()) this->msgHeartStack.pop();
        }
        else{//文本信息，写入剪贴板
            Log(tr("接收到移动端文本消息：")+lastMsg);
            QClipboard *board = QApplication::clipboard();
            board->setText(lastMsg);msgList.append(lastMsg);
            //自动回复: R\n
            this->socket->write(RESPONSE,strlen(RESPONSE));
        }

    }

}

void msgServer::sendMsg(QString msg){

    if(this->socket!=nullptr){
        //将换行符用其他字符替代
        msg = msg.replace("\r",REPLACER);
        msg = msg.replace("\n",REPLACEN);
        msg = msg + "\n";
        std::string temp = msg.toStdString();
        this->socket->write(temp.c_str(),strlen(temp.c_str()));
    }

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
        Log(QString("消息服务器：监听成功，端口：%1").arg(QString::number(port)));
        this->listeningPort = port;
    }else{
        Log(QString("消息服务器：监听失败，端口：%1").arg(QString::number(port)));
    }
    return ok;
}


bool msgServer::ifConnected(){
    return this->socket!=nullptr;
}

QString msgServer::getConnection(){
    return ifConnected()?("连接至"+getIPv4(this->socket->peerAddress().toIPv4Address())):"无连接";
}

int errorTimes = 0;
void msgServer::timerEvent(QTimerEvent *e){

    /**检查连接是否正常*/
    if(checkClientAliveTimerID == e->timerId()){
        //如果已经连接，则heartbeat：每 2 秒自动回复: R\n
        if(this->socket!=nullptr){
          //  qDebug("State:%d",this->socket->state()); //3是正常的状态：connected
            if(this->socket->state()!=3) {errorTimes++; if(errorTimes==2){this->socket->close();errorTimes=0;} Log("检测到连接异常-1，自动断开"); }
            this->socket->write(RESPONSE,strlen(RESPONSE));
            this->socket->waitForBytesWritten();
            msgHeartStack.push(0); if(msgHeartStack.size()>2){this->socket->close();errorTimes=0;msgHeartStack.clear(); Log("检测到连接异常-2，自动断开"); }
        }else if(ipList.size()!=0){
            //如果还没连接：发送ip、端口号、连接密钥的信息
            for(int i=0;i<ipList.size();i++){
                netInfoStr = ipList.at(i)+tr(" ")+conf->getConfig("port")+tr(" ");
                QHostAddress host  = QHostAddress(brocastList.at(i));
                QString info = encrypt(netInfoStr+conf->getConfig("secret"));
                std::string strTemp= info.toStdString();
                const char* temp = strTemp.c_str();
                //qDebug()<<"发送UDP报文："<<netInfoStr+conf->getConfig("secret")+"  加密后："<<temp;
                this->udpSocket->writeDatagram(temp,strlen(temp),host,56789);
                this->udpSocket->waitForBytesWritten();
            }
        }
    }

    /**检查网卡信息是否变化*/
    if(checkIPClientTimerID == e->timerId()){
        getLanBrocastAddress_1();
        if(ipList_Curr.size()!=ipList.size()){/**发送信号使UI更新托盘信息*/ emit(ipChange()); Log(tr("网卡数量改变，更新托盘ToolTip")); }
        else {
            bool flag = true;
            for(int i=0;i<ipList_Curr.size();i++){
                bool flag_1 = false;
                for(int j=0;j<ipList.size();j++){
                    if(ipList_Curr.at(i)==ipList.at(j)) {flag_1 = true; break;}
                }
                flag = flag & flag_1;
            }
            /**发送信号使UI更新托盘信息*/
            if(!flag) {emit(ipChange()); Log(tr("网卡IP改变，更新托盘ToolTip"));}
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

void  msgServer::getLanBrocastAddress(){
    ipList.clear(); brocastList.clear();  networkcardList.clear();
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    Log(tr("获取网卡信息："));
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
         Log(tr("网卡名称：")+networkCardName);


         // 读取一个IP地址列表
        QList<QNetworkAddressEntry> entryList = interf.addressEntries();
         for(int j = 0; j < entryList.count(); j++)
         {

             QNetworkAddressEntry entry = entryList.at(j);

             QString ip = entry.ip().toString();                             //IP地址
             QString mask = entry.netmask().toString();            //子网掩码
             QString bcast = entry.broadcast().toString();          //广播地址
             if(bcast.compare("")==0) continue;

             ipList.append(ip); brocastList.append(bcast); networkcardList.append(networkCardName);

             Log(tr("ip：")+ip+tr("  掩码：")+mask+"  广播地址："+bcast);

         }

     }
}

void  msgServer::getLanBrocastAddress_1(){
    ipList_Curr.clear();
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
                ipList_Curr.append(ip);
         }

     }


}

