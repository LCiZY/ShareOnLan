#ifndef FILESERVER_H
#define FILESERVER_H

#include <QObject>
#include<QTcpSocket>
#include<QTcpServer>
#include<QApplication>
#include<QClipboard>
#include<QTimerEvent>
#include<QTimer>
#include<QFile>
#include<QIODevice>
#include<QFileDialog>
#include<QStandardPaths>
#include<QMessageBox>
#include<QThread>

#include<globaldata.h>





class FileSocket : public QTcpSocket
{
    Q_OBJECT
public:
    FileSocket(int socketdesc,QTcpSocket *parent = NULL);
public slots:
    void receiveFile();
    void sendFile(FileInfo *);
signals:
    void readProgress(qint64);
    void fileTransferDone();
    void currFileInfo(qint64 fileSize, QString fileName);

};




class fileServer:public QTcpServer
{
    Q_OBJECT
public:
    fileServer();

public:
    const int fileServerListeningPort = 65534;
    bool ifConnected();
    bool listenOn(int port);
    void closeSocket();
    void serverShutDown();
    void setSendFileInfo(FileInfo *fileInfo);
    bool ifSend;

    void sendFile(FileInfo *sendFileInfo);
    void receiveFile();

signals:
    void fileSend(FileInfo *);
    void receiveProgress(qint64);
    void sendProgress(qint64);
    void newFileConnection();
    void fileTransferDone();
    void currFileInfo(qint64 fileSize, QString fileName);

public slots:
    void socketDisconnect();

protected:

    void incomingConnection(int descriptor);

private:

    FileSocket* fileSocket;
    QThread* sendFileThread;
    QThread* receiveFileThread;

    FileInfo *sendFileInfo;

};





#endif // FILESERVER_H
