#include "fileserver.h"
#include<exception>


fileServer::fileServer()
{
    ifSend = false;
    int port = conf->getConfig("fileServerPort").toInt();
    if(PORT_BOTTOM <= port && port <= PORT_TOP){
        fileServerListeningPort = port;
    }

}

bool fileServer::ifConnected(){
    bool rt = false;
    QHashIterator<int, FileSocket*> iter(this->descriptor2socket);
    while(iter.hasNext()) {
        iter.next();
        if(!iter.value()->isTransferDone())
            rt = true;

    }
    return rt;
}

bool fileServer::listenOn(int port){

    close();
    bool ok;
    if((ok = listen(QHostAddress::Any,port))){
        Log(QString("文件服务器：监听成功，端口：%1").arg(QString::number(port)));
    }else{
        Log(QString("文件服务器：监听失败，端口：%1").arg(QString::number(port)));
    }
    return ok;

}

void fileServer::clearSockets(){
//    QHashIterator<int, FileSocket*> iter(this->descriptor2socket);
//    while(iter.hasNext()) {
//        iter.next();
//        FileSocket* socket = iter.value();
//        if(!socket->isTransferDone()){

//            Log(tr("false"));
//            Log(tr(socket->isValid()? "valid" :"invalid"));
//            Log(tr(socket->isReadable()? "readable" :"unreadable"));
//            Log(tr(socket->isWritable()? "writable" :"unwritable"));
//            socket->deleteLater();

//            Log(tr("over"));
//        }
//    }
    this->descriptor2socket.clear();
}


//断开连接，关闭socket
void fileServer::closeSocket(){
    clearSockets();
}

//服务停止，停止监听端口
void fileServer::serverShutDown(){
    clearSockets();
    this->close();
}


void fileServer::incomingConnection(int descriptor){
    FileSocket* socket = new FileSocket(descriptor);
    incomingConnection(socket);
}

void fileServer::incomingConnection(FileSocket* socket){

    Log(tr("接收到文件传输连接:")+QString::number(socket->socketDescriptor()));
    initSocket(socket);

    if(ifSend){  //发送文件
        sendFile(socket);
    }else{  //接收文件
        receiveFile(socket);
    }

    descriptor2socket[socket->socketDescriptor()] = socket;
    //告知UI显示进度条界面
    emit newFileConnection();

}


void fileServer::socketDisconnect(){
    Log(tr("文件传输连接关闭。"));
}


void fileServer::initSocket(FileSocket* socket){
    socket->setSocketOption(QAbstractSocket::KeepAliveOption,1); //设置keepalive连接
    connect(socket,SIGNAL(fileTransferDone()),this,SIGNAL(fileTransferDone()));
    connect(socket,SIGNAL(currFileInfo(qint64,QString)),this,SIGNAL(currFileInfo(qint64,QString)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(socketDisconnect()));
    connect(socket,&FileSocket::fileTransferDone,[=](){
        descriptor2socket.remove(socket->socketDescriptor());
       });
}

/**
*
*   接收文件：在子线程中运行
*   描述：1.接收
*         2.保存到用户指定位置或默认位置
*
*
*/

void fileServer::receiveFile(FileSocket* socket){
    Log(tr("接收文件"));
    socket->setFileInfo(receiveFilesQueue.dequeue());
    //设置准备接收文件的大小
    if(socket->getFileInfo() == nullptr) { Log("fileServer::receiveFile：未知的接收文件信息（1）"); socket->close();return; }

    QThread* receiveFileThread = new QThread();
    //连接数据到来时的槽函数
    connect(socket,SIGNAL(readyRead()),socket,SLOT(receiveFile()));

    //先将filesocket删除再退出线程（在线程的事件循环结束前调用filesocket的deletelater
   // connect(socket,&FileSocket::disconnected,socket,[=](){socket->deleteLater();});
    connect(socket,&FileSocket::destroyed,[=](){
                                                receiveFileThread->quit();
                                                receiveFileThread->deleteLater();
                                                if(receiveFilesQueue.isEmpty())
                                                    emit fileTransferDone();
                                               });

    //接收进度
    connect(socket,SIGNAL(readProgress(qint64)),this,SIGNAL(receiveProgress(qint64)));

    //移动到子线程并开始
    socket->moveToThread(receiveFileThread);
    receiveFileThread->start();
}

/**
*
*   发送文件:在子线程中运行
*   描述：1.用户选择文件
*         2.尝试打开文件
*           3.发送文件名
*              4.发送文件
*               5.关闭连接
*/
void fileServer::sendFile(FileSocket* socket){
    Log(tr("发送文件"));
    ifSend = false;
    socket->setFileInfo(sendFilesQueue.dequeue());
    if(socket->getFileInfo() == nullptr) { Log("fileServer::sendFile：未知的接收文件信息（2）"); socket->close();return; }

    QThread* sendFileThread = new QThread();
    //连接发送文件时的槽函数
    connect(this,SIGNAL(fileSend()),socket,SLOT(sendFile()));

    //先将filesocket删除再退出线程（在线程的事件循环结束前调用filesocket的deletelater
    //connect(fileSocket,&FileSocket::disconnected,fileSocket,[=](){fileSocket->deleteLater();});
    connect(socket,&FileSocket::destroyed,[=](){
        sendFileThread->quit();
        sendFileThread->deleteLater();
        if(sendFilesQueue.isEmpty())
            emit fileTransferDone();
        else
            emit sendNextFile();
    });

    //发送进度
    connect(socket,SIGNAL(bytesWritten(qint64)),this,SIGNAL(sendProgress(qint64)));

    //移动到子线程并开始
    socket->moveToThread(sendFileThread);
    sendFileThread->start();
    Sleep(10); emit fileSend();

}








/*
 *
 *
 *
 *
 * File Socket
 *
 *
 *
 *
 */

FileSocket::FileSocket(int socketdesc, QTcpSocket *){
    this->setSocketDescriptor(socketdesc);
    this->setFileInfo(nullptr);
    transferDone = false;
    detectConnectionTimerID = startTimer(1000);
    lastTransferTime = clock();
}


FileSocket::~FileSocket(){
    killTimer(this->detectConnectionTimerID);
    delete this->fileInfo;
}


void FileSocket::sendFile(){

    QString fp = this->getFileInfo()->filePath;
    QFile sendfile(fp);
    if(!sendfile.exists()){Log("FileSocket::sendFile：文件不存在。"); return;}
    if(!sendfile.open(QIODevice::ReadOnly)){
        Log("错误,文件打开失败，请重试");
        this->close();
        return;
    }

    emit currFileInfo(sendfile.size(), fp);

    char *buf= new char[FILESENDBUFFERSIZE];
    qint64 length = 1;
    //耗时操作:发送文件
    int count=0;
    while ((length=sendfile.read(buf,FILESENDBUFFERSIZE))!=-1&&length!=0) {
        this->write(buf,length);
        count+=length;
        this->waitForBytesWritten();
        lastTransferTime = clock();
    }
    delete buf;

    Log("文件发送完成，发送大小："+ QString::number(count));
    sendfile.close();
    this->close();
    transferDone = true;
    emit fileTransferDone();
    this->deleteLater();
}



void FileSocket::receiveFile(){


    FileInfo* fileInfo = this->getFileInfo();

    QString fileSaveDir = conf->getConfig("fileReceiveLocation") == "" ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : conf->getConfig("fileReceiveLocation");

    //读取用户默认保存位置
    QString userChooseFileName = fileSaveDir + tr("/") + (fileInfo->fileName==""?tr("untitle.file"):fileInfo->fileName);

    QFile receivefile(userChooseFileName);

    if(fileInfo->resolveFileSize==0){
        Log(tr("开始接收文件,文件大小:%1 文件名:%2").arg(QString::number(fileInfo->fileSize)).arg(fileInfo->fileName));
        emit currFileInfo(fileInfo->fileSize, fileInfo->fileName);
    }


    if(receivefile.open(fileInfo->resolveFileSize==0?QIODevice::WriteOnly:QIODevice::Append)){

        char *buf=new char[FILEBUFFERSIZE];
        qint64 length = 0;

        //耗时操作，接收文件
        while ((length=this->read(buf,FILEBUFFERSIZE))!=-1 && length != 0) {
            // qDebug() << "length1: " << length;
            receivefile.write(buf,length);
            fileInfo->resolveFileSize+=length;
            emit readProgress(length);
            lastTransferTime = clock();
        }
        delete buf;

        receivefile.close();
    }else{
        //接收失败
        Log(tr("接收失败:文件打开失败"));
    }


    if(fileInfo->resolveFileSize >= fileInfo->fileSize) {
        //接收完成
        Log(tr("接收文件完成,接收大小:%1, 文件保存位置:%2").arg(QString::number(fileInfo->resolveFileSize)).arg(userChooseFileName));

        this->close();

        transferDone = true;
        emit fileTransferDone();
        this->deleteLater();
    }


}


void FileSocket::setFileInfo(FileInfo* fileInfo){
     this->fileInfo = fileInfo;
}

FileInfo* FileSocket::getFileInfo(){
    return this->fileInfo;
}

bool FileSocket::isTransferDone(){
    return this->transferDone;
}


void FileSocket::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == this->detectConnectionTimerID) {
        if(clock() - lastTransferTime > TRANSFERTIMEOUT){
             if(fileInfo != nullptr)
                Log(tr("***接收文件失败***接收大小:%1, 实际大小: %2, 文件名称:%3").arg(QString::number(fileInfo->resolveFileSize)).arg(QString::number(fileInfo->fileSize)).arg(fileInfo->fileName));
             emit fileTransferDone();
             this->transferDone = true;
             this->deleteLater();
        }
    }
}




