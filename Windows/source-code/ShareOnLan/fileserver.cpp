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

    listenOn(fileServerListeningPort);
    ifSend = false;
}

bool fileServer::ifConnected(){

    return this->fileSocket!=nullptr;

}

void fileServer::listenOn(int port){

    close();
    listen(QHostAddress::Any,port);


}

//断开连接，关闭socket
void fileServer::closeSocket(){
     if(this->fileSocket!=nullptr)this->fileSocket->close();
}

//服务停止，停止监听端口
void fileServer::serverShutDown(){
    if(this->fileSocket!=nullptr)this->fileSocket->close();
    this->close();
}


void fileServer::incomingConnection(int descriptor){
    if(this->fileSocket!=nullptr) { return; }
    pauseAccepting();
    qDebug()<<"接收到文件传输连接:"<<descriptor;
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
//emit currFileInfo(receiveFilesSizeQueue.head().toInt(),receiveFilesNameQueue.head());

void fileServer::socketDisconnect(){
    if( this->fileSocket!=nullptr){
          qDebug()<<"文件传输连接关闭:"<<this->fileSocket->socketDescriptor();
           Log(tr("文件传输连接关闭:")+QString::number(this->fileSocket->socketDescriptor()));
        this->fileSocket->deleteLater();
        this->fileSocket = nullptr;
        this->sendFileThread = nullptr;
        this->receiveFileThread = nullptr;
        //恢复监听
        resumeAccepting();
    }
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
    this->setFileSize(receiveFilesSizeQueue.head().toInt());
    receiveSize = 0;
    receiveFileThread = new QThread(this->fileSocket);
    connect(fileSocket,SIGNAL(readyRead()),fileSocket,SLOT(receiveFile()));
    connect(fileSocket,SIGNAL(disconnected()),receiveFileThread,SLOT(quit()));
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

    sendFileThread = new QThread(this->fileSocket);
    connect(this,SIGNAL(fileSend(QString)),fileSocket,SLOT(sendFile(QString)));
    connect(fileSocket,SIGNAL(disconnected()),sendFileThread,SLOT(quit()));
    connect(fileSocket,SIGNAL(bytesWritten(qint64)),this,SIGNAL(sendProgress(qint64)));
    //移动到子线程并开始
    fileSocket->moveToThread(sendFileThread);
    sendFileThread->start();
    Sleep(10); emit fileSend(fileName);
}


void FileSocket::sendFile(QString userChooseFileName){


    QFile sendfile(userChooseFileName);
    if(!sendfile.open(QIODevice::ReadOnly)){
        Log("错误,文件打开失败，请重试");
        this->close();
        return;
    }

    emit currFileInfo((int)sendfile.size(),userChooseFileName);

    char buf[FILEBUFFERSIZE];
    qint64 length = 1;
    //耗时操作:发送文件
    while ((length=sendfile.read(buf,FILEBUFFERSIZE))!=-1&&length!=0) {
        this->write(buf,length);
        this->waitForBytesWritten();
    }
    Log("文件发送完成");
    qDebug()<<"文件发送完成";
    sendfile.close();
    this->close();

    emit fileTransferDone();
}



void FileSocket::receiveFile(){

    //读取用户默认保存位置
     QString userChooseFileName = conf->getConfig("fileReceiveLocation")
                +tr("/")+(receiveFilesNameQueue.head().compare("")==0?tr("untitle.file"):receiveFilesNameQueue.head());

    if(userChooseFileName.compare("")==0){//用户没有设置过路径
        userChooseFileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
                +tr("/")+(receiveFilesNameQueue.head().compare("")==0?tr("untitle.file"):receiveFilesNameQueue.head());
    }

    QFile receivefile(userChooseFileName);
    if(receiveSize==0){
        qDebug()<<"开始接收文件,文件大小："<<receiveFilesSizeQueue.head()<<"  文件名："<<receiveFilesNameQueue.head();
        Log(tr("开始接收文件,文件大小：")+receiveFilesSizeQueue.head()+tr("  文件名：")+receiveFilesNameQueue.head());
    }

        if(receiveSize==0) emit currFileInfo(receiveFilesSizeQueue.head().toInt(),receiveFilesNameQueue.head());
        if(receivefile.open(receiveSize==0?QIODevice::WriteOnly:QIODevice::Append)){

            char buf[FILEBUFFERSIZE];
            qint64 length=1;

            //耗时操作，接收文件
            while ((length=this->read(buf,FILEBUFFERSIZE))!=-1&&length!=0) {
                receivefile.write(buf,length);
                receiveSize+=length;
                emit readProgress(length);
            }
            //接收完成
            if(receiveSize==fileSize){
                 qDebug()<<"接收完成,文件保存位置:"<<userChooseFileName;
                 Log(tr("接收完成，文件保存位置:")+userChooseFileName);
                 this->close();
                 receiveFilesNameQueue.dequeue();
                 receiveFilesSizeQueue.dequeue();
            }

            receivefile.close();
            if(receiveFilesNameQueue.size()==0) emit fileTransferDone();
            return;
        }

    //接收失败
    qDebug()<<"接收失败";
    Log(tr("接收失败"));

}




void fileServer::setFileSize(int size){
    fileSize = size;
}

void fileServer::setSendFileName(QString filename){
    sendFileName = filename;

}







