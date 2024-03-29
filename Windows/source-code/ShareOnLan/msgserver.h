#ifndef MSGSERVER_H
#define MSGSERVER_H

#include <QObject>
#include<QUdpSocket>
#include<QTcpSocket>
#include<QTcpServer>
#include<QApplication>
#include<QClipboard>
#include<QTimerEvent>
#include<QTimer>
#include<globaldata.h>
#include<QHostInfo>
#include<QNetworkInterface>
#include<QStack>
#include <QMutex>


class msgServer : public QTcpServer
{
    Q_OBJECT
public:
    msgServer(QObject* parent=0,int port=0);
    ~msgServer();
    void timerEvent(QTimerEvent *event);

    const QString REPLACER="${r}";
    const QString REPLACEN="${n}";
    const char* RESPONSE = "R\n";
    const char* FILEINFORESPONSE="FILEINFO R\n";
    const int timeOutValue = 10;
    const int DEFAULT_IP_PORT_UDP_PORT = 56789;
    QStack<int> msgHeartStack;

    int listeningPort;

    bool ifConnected();
    void closeSocket();
    void getLanBrocastAddress();
    const QTcpSocket* getSocket();

    void incomingConnection(QTcpSocket* socket);

public slots:
    void readMsg();
    void send(const char *);
    void sendMsg(QString);
    void socketDisconnect();
    QString getConnection();
    bool listenOn(int port);
    void serverShutDown();



signals:
    void clientChange();
    void ipChange();
    void otherPCReadyReceiveFile();

protected:
    void incomingConnection(int descriptor);

private:
    QTcpSocket *socket;
    QUdpSocket* udpSocket;


    QStringList msgList;
    QString lastMsg;
    QString msg;
    QString netInfoStr;


    int checkClientAliveTimerID;
    int checkIPClientTimerID;

    QString encrypt(QString);
    QString getIPv4(qint32 ip);
    void getCurrIPList();



};

#endif // MSGSERVER_H
