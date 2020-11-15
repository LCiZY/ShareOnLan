#include "msgserver.h"

msgServer::msgServer(QObject *, int port)
{
    this->listenOn(port);
    this->socket = nullptr;
    this->udpSocket = new QUdpSocket;
    this->msg = "";


    getLanBrocastAddress();
    checkClientAliveTimerID = startTimer((timeOutValue-2)*1000);
}

msgServer::~msgServer(){
    socket->deleteLater();
}


void msgServer::incomingConnection(int descriptor){

    if(this->socket!=nullptr){qDebug()<<"已存在其他TCP连接！拒绝连接";Log(tr("已存在其他TCP连接！拒绝连接"));  return;}
    pauseAccepting();
    this->socket = new QTcpSocket(this);
    this->socket->setSocketDescriptor(descriptor);
    this->socket->setSocketOption(QAbstractSocket::KeepAliveOption,1); //设置keepalive连接
    if(!this->socket->waitForConnected()) {qDebug()<<"错误："<<this->socket->errorString();Log(this->socket->errorString());}
    connect(socket,SIGNAL(readyRead()),this,SLOT(readMsg()));
    connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnect()));

    qDebug()<<"接受socket连接："<<descriptor<<"ip:"<<getIPv4(socket->peerAddress().toIPv4Address())<<"   port:"<<socket->peerPort();

    Log(tr("接受socket连接：")+QString::number(descriptor)+tr("  ip:")+getIPv4( socket->peerAddress().toIPv4Address())+tr("  port:")+QString::number(socket->peerPort()));
    //发送client改变消息
    emit(clientChange());
}

void msgServer::socketDisconnect(){

    if(this->socket!=nullptr){
        qDebug()<<"socket断开："<<this->socket->socketDescriptor();
        Log(tr("socket断开：")+QString::number(this->socket->socketDescriptor()));
        disconnect(socket,SIGNAL(readyRead()),this,SLOT(readMsg()));
        disconnect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnect()));
        this->socket->deleteLater();
        this->socket = nullptr;

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



        qDebug()<<"接收到移动端："<<lastMsg;
        Log(tr("接收到移动端：")+lastMsg);

        if(lastMsg.indexOf("${FILEINFO}")==0){//如果是文件信息
            QStringList splices = lastMsg.split(" ");
            for(int i=1;i+1<splices.size();i+=2){
                if(splices.at(i)=="fileName")      receiveFilesNameQueue.enqueue(splices.at(i+1));
                else if(splices.at(i)=="fileSize") receiveFilesSizeQueue.enqueue(splices.at(i+1));
            }
            //自动回复: FILEINFO R\n
            this->socket->write(FILEINFORESPONSE,strlen(FILEINFORESPONSE));
        }else{//剪贴板信息，写入剪贴板

            QClipboard *board = QApplication::clipboard();
            board->setText(lastMsg);msgList.append(lastMsg);
            //自动回复: R\n
            this->socket->write(RESPONSE,strlen(RESPONSE));
        }

    }

}

void msgServer::sendMsg(QString msg){

    //将换行符用其他字符替代
    msg = msg.replace("\r",REPLACER);
    msg = msg.replace("\n",REPLACEN);

    msg = msg + "\n";
    if(this->socket!=nullptr){
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
void msgServer::listenOn(int port){
    close();
    listen(QHostAddress::Any,port);
    this->listeningPort = port;
}


bool msgServer::ifConnected(){
    return this->socket!=nullptr;
}

QString msgServer::getConnection(){
    return ifConnected()?("Connect With "+getIPv4(this->socket->peerAddress().toIPv4Address())):"No Connection";
}

int errorTimes = 0;
int timeEventPlayTimes = 0;
void msgServer::timerEvent(QTimerEvent *){
    //每8秒自动回复: R\n
    if(this->socket!=nullptr){
      //  qDebug("State:%d",this->socket->state()); //3是正常的状态：connected
        if(this->socket->state()!=3) {errorTimes++; if(errorTimes==2){this->socket->close();errorTimes=0;} Log("检测到连接异常，自动断开"); }
        this->socket->write(RESPONSE,strlen(RESPONSE));
        this->socket->waitForBytesWritten();
    }else if(ipList.size()!=0){
        if(timeEventPlayTimes==ipList.size()){ timeEventPlayTimes=0; }
        netInfoStr = ipList.at(timeEventPlayTimes%ipList.size())+tr(" ")+conf->getConfig("port")+tr(" "); timeEventPlayTimes++;
        QHostAddress host  = QHostAddress(netInfo[1]);
        QString info = encrypt(netInfoStr+conf->getConfig("secret"));
        std::string strTemp= info.toStdString();
        const char* temp = strTemp.c_str();
      //  qDebug()<<"发送UDP报文："<<netInfoStr+conf->getConfig("secret")+" "<<temp;
        this->udpSocket->writeDatagram(temp,strlen(temp),host,56789);
        this->udpSocket->waitForBytesWritten();
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
    ipList.clear();
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
                netInfo[0]=ip; netInfo[1]=bcast;
             ipList.append(ip);
          //   qDebug()<<"ip:"<<ip<<"   掩码："<<mask<<"   广播地址:"<<bcast;
             Log(tr("ip：")+ip+tr("  掩码：")+mask+"  广播地址："+bcast);

         }

     }


}

