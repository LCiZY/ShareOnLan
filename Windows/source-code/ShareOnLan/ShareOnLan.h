#ifndef SHAREONLAN_H
#define SHAREONLAN_H

#include<QMenu>
#include<QAction>
#include<QSystemTrayIcon>
#include<QCloseEvent>
#include<QMouseEvent>
#include<QIntValidator>
#include<QDesktopServices>
#include<QTime>
#include<QUrl>
#include<QtMath>
#include<QPainter>
#include<QKeyEvent>
#include<QPainterPath>
#include<QRectF>
#include<QPolygon>
#include<QRegion>
#include<QMimeData>
#include<QNetworkProxy>
#include<QLocalSocket>
#include<QLocalServer>
#include<QTextStream>
#include<QGraphicsDropShadowEffect>
#include<QPixmap>


#include<msgserver.h>
#include<fileserver.h>
#include<component_ui.h>



namespace Ui {
class ShareOnLan;
}

class ShareOnLan : public DraggableWidget
{
    Q_OBJECT

public:
    explicit ShareOnLan(QWidget *parent = 0);
    ~ShareOnLan();

protected:
    void closeEvent(QCloseEvent* event);
    virtual void keyPressEvent(QKeyEvent *ev);
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);

public slots:
    void clientChange();
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason);
    void on_SendToPhone();
    void on_SendFile();
    void on_ShowNetInfo();
    void on_restartServer();
    void on_connectTo();
    void on_showSettingAction();

    void on_exitAppAction();

    void portChange(QString);
    void secretChange(QString);
    void ipChange();
    void otherPCReadyReceiveFile();
    void slot_checkBox_ifhidewhenlaunch(bool);
    void slot_checkBox_autoLaunch(bool);
    void showMain();
    void showMini();
    void hideWindow();
    void changeFileReceiveLocation();
    void openFileLocationFolder();
    void triggerSendFile();

    void showProgressUI();
    void setProgressInfo(qint64 fileSize, QString fileName);
    void progressUIChange(qint64);
    void progressUIDestroy();

    void newLocalSocketConnection();
    void connectToOtherPC(QString ip, quint16 port);
private:
    Ui::ShareOnLan *ui;
    QSystemTrayIcon* mSysTrayIcon;
    QMenu *mMenu;
    QAction *mSendToPhoneAction;
    QAction *mSendFile;
    QAction *mConnectInfoAction;
    QAction *mRestartServiceAction;
    QAction *mConnectToAction;
    QAction *mSettingAction;
    QAction *mExitAppAction;
    QIntValidator *valid_port;
    QLocalServer* m_localServer;
    ProgressUI* progressui;
    Connect2UI* connect2ui;


    msgServer* server;
    fileServer* fileserver;
public:
    bool listening();
    void serverInit();
    void windowInit();
    void sysTrayIconInit();
    void sysTrayMenuInit();
    void varInit();
    void setWinFlags();
    void clearConnection();
    void sysTrayTextChange();

    void setLocalServer( QLocalServer* m_localServer);

private:
    QString tray_tooltip;

};

class SOLSingleInstanceDetecter : public QObject
{
    Q_OBJECT

public:
    QString serverName = "ShareOnLanSingleInstanceServer";
    bool detectSingleInstance();
    bool buildLocalServer();
    QLocalServer* m_localServer;
};

#endif // SHAREONLAN
