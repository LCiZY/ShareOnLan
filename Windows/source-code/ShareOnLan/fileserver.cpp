#include "fileserver.h"
#include<exception>

FileSocket::FileSocket(int socketdesc, QTcpSocket *){
    this->setSocketDescriptor(socketdesc);
}

fileServer::fileServer()
{
    this->fileSocket = nullptr;
    this->sendFileThread = nullptr;
    this->receiveFileThread = nullptr;

    ifSend = false;
}

bool fileServer::ifConnected(){

    return this->fileSocket!=nullptr;

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



//断开连接，关闭socket
void fileServer::closeSocket(){
     if(this->fileSocket!=nullptr){
         this->fileSocket->close();
         this->fileSocket->deleteLater();
     }
}

//服务停止，停止监听端口
void fileServer::serverShutDown(){
    if(this->fileSocket!=nullptr)this->fileSocket->close();
    this->close();
}


void fileServer::incomingConnection(int descriptor){

    if(this->fileSocket!=nullptr) { return; }
    pauseAccepting();
    Log(tr("接收到文件传输连接:")+QString::number(descriptor));
    this->fileSocket = new FileSocket(descriptor);
    this->fileSocket->setSocketOption(QAbstractSocket::KeepAliveOption,1); //设置keepalive连接
        connect(fileSocket,SIGNAL(fileTransferDone()),this,SIGNAL(fileTransferDone()));
        connect(fileSocket,SIGNAL(currFileInfo(int,QString)),this,SIGNAL(currFileInfo(int,QString)));
        connect(fileSocket,SIGNAL(disconnected()),this,SLOT(socketDisconnect()));

    if(ifSend){  //发送文件
        sendFile(sendFileName);
        ifSend = false;
    }else{  //接收文件
        receiveFile();
    }
    //告知UI显示进度条界面
    emit newFileConnection();

}


void fileServer::socketDisconnect(){
        Log(tr("文件传输连接关闭。"));
        if(!receiveFilesNameQueue.isEmpty()&&!receiveFilesSizeQueue.isEmpty()){
            receiveFilesNameQueue.dequeue();
            receiveFilesSizeQueue.dequeue();
            if(receiveFilesNameQueue.size()==0) emit fileTransferDone();
        }
        //恢复监听
        resumeAccepting();
}



/**
*
*   接收文件：在子线程中运行
*   描述：1.接收
*         2.保存到用户指定位置或默认位置
*
*
*/

void fileServer::receiveFile(){
    //设置准备接收文件的大小
    if(receiveFilesSizeQueue.size()==0||receiveFilesNameQueue.size()==0) {Log("fileServer::receiveFile：文件大小或文件名未知-1");this->fileSocket->close();return; }
    this->setFileSize(receiveFilesSizeQueue.head().toInt());
    //累积接收大小
    receiveSize = 0;
    receiveFileThread = new QThread();
    //连接数据到来时的槽函数
    connect(fileSocket,SIGNAL(readyRead()),fileSocket,SLOT(receiveFile()));

    //先将filesocket删除再退出线程（在线程的事件循环结束前调用filesocket的deletelater
    connect(fileSocket,&FileSocket::disconnected,fileSocket,[=](){fileSocket->deleteLater();});
    connect(fileSocket,&FileSocket::destroyed,[=](){ receiveFileThread->quit();receiveFileThread->deleteLater(); this->fileSocket = nullptr;this->receiveFileThread = nullptr; });

    //接收进度
    connect(fileSocket,SIGNAL(readProgress(qint64)),this,SIGNAL(receiveProgress(qint64)));

    //移动到子线程并开始
    fileSocket->moveToThread(receiveFileThread);
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
void fileServer::sendFile(QString fileName){

    sendFileThread = new QThread();
    //连接发送文件时的槽函数
    connect(this,SIGNAL(fileSend(QString)),fileSocket,SLOT(sendFile(QString)));

    //先将filesocket删除再退出线程（在线程的事件循环结束前调用filesocket的deletelater
    connect(fileSocket,&FileSocket::disconnected,fileSocket,[=](){fileSocket->deleteLater();});
    connect(fileSocket,&FileSocket::destroyed,[=](){ sendFileThread->quit();sendFileThread->deleteLater(); this->fileSocket = nullptr;this->sendFileThread = nullptr; });

    //发送进度
    connect(fileSocket,SIGNAL(bytesWritten(qint64)),this,SIGNAL(sendProgress(qint64)));

    //移动到子线程并开始
    fileSocket->moveToThread(sendFileThread);
    sendFileThread->start();
    Sleep(10); emit fileSend(fileName);

}



void FileSocket::sendFile(QString userChooseFileName){


    QFile sendfile(userChooseFileName);
    if(!sendfile.exists()){Log("FileSocket::sendFile：文件不存在。"); return;}
    if(!sendfile.open(QIODevice::ReadOnly)){
        Log("错误,文件打开失败，请重试");
        this->close();
        return;
    }

    emit currFileInfo((int)sendfile.size(),userChooseFileName);

    char *buf= new char[FILESENDBUFFERSIZE];
    qint64 length = 1;
    //耗时操作:发送文件
    int count=0;
    while ((length=sendfile.read(buf,FILESENDBUFFERSIZE))!=-1&&length!=0) {
        this->write(buf,length);
        count+=length;
        this->waitForBytesWritten();
    }
    delete buf;

    Log("文件发送完成，发送大小："+ QString::number(count));
    sendfile.close();
    this->close();

    emit fileTransferDone();
}



void FileSocket::receiveFile(){

    if(receiveFilesSizeQueue.size()==0||receiveFilesNameQueue.size()==0)
    {Log("fileServer::receiveFile：文件大小或文件名未知-2");this->close(); return; }


    //读取用户默认保存位置
     QString userChooseFileName = conf->getConfig("fileReceiveLocation")
                +tr("/")+(receiveFilesNameQueue.head().compare("")==0?tr("untitle.file"):receiveFilesNameQueue.head());

    if(userChooseFileName.compare("")==0){//用户没有设置过路径
        userChooseFileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
                +tr("/")+(receiveFilesNameQueue.head().compare("")==0?tr("untitle.file"):receiveFilesNameQueue.head());
    }

    QFile receivefile(userChooseFileName);
    if(receiveSize==0){
        Log(tr("开始接收文件,文件大小:%1 文件名:%2").arg(receiveFilesSizeQueue.head()).arg(receiveFilesNameQueue.head()));
        emit currFileInfo(receiveFilesSizeQueue.head().toInt(),receiveFilesNameQueue.head());
    }

    if(receivefile.open(receiveSize==0?QIODevice::WriteOnly:QIODevice::Append)){

            char *buf=new char[FILEBUFFERSIZE];
            qint64 length=1;

            //耗时操作，接收文件
            while ((length=this->read(buf,FILEBUFFERSIZE))!=-1&&length!=0) {
                receivefile.write(buf,length);
                receiveSize+=length;
                emit readProgress(length);
            }
            delete buf;
            //接收完成
            if(receiveSize>=fileSize){
                 Log(tr("接收文件完成,接收大小:%1 文件保存位置:%2").arg(QString::number(receiveSize)).arg(userChooseFileName));
                 this->close();
            }

            receivefile.close();

            return;
     }

    //接收失败
    Log(tr("接收失败:文件打开失败"));

}




void fileServer::setFileSize(int size){
    fileSize = size;
}

void fileServer::setSendFileName(QString filename){
    sendFileName = filename;

}







