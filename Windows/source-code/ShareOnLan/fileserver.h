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
#include<time.h>




class FileSocket : public QTcpSocket
{
    Q_OBJECT
public:
    FileSocket(int socketdesc, QTcpSocket *parent = NULL);
    ~FileSocket();

    virtual void timerEvent( QTimerEvent *event);

    void setFileInfo(FileInfo* fileInfo);
    FileInfo* getFileInfo();
    bool isTransferDone();

public slots:
    void receiveFile();
    void sendFile();

signals:
    void readProgress(qint64);
    void fileTransferDone();
    void currFileInfo(qint64 fileSize, QString fileName);
private:
    FileInfo* fileInfo;
    bool transferDone;

    int detectConnectionTimerID;
    clock_t lastTransferTime;
};




class fileServer:public QTcpServer
{
    Q_OBJECT
public:
    fileServer();

public:
    void incomingConnection(FileSocket* socket);

    const int DEFAULT_FILE_SERVER_PORT = 65534;
    int fileServerListeningPort = DEFAULT_FILE_SERVER_PORT;
    bool ifConnected();
    bool listenOn(int port);
    void closeSocket();
    void serverShutDown();
    bool ifSend;

    void initSocket(FileSocket* socket);
    void sendFile(FileSocket* socket);
    void receiveFile(FileSocket* socket);

signals:
    void fileSend();
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
    void clearSockets();
    QHash<int, FileSocket*> descriptor2socket;

};





#endif // FILESERVER_H
