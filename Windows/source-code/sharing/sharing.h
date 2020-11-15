#ifndef SHARING_H
#define SHARING_H

#include <QMainWindow>
#include<QMenu>
#include<QAction>
#include<QSystemTrayIcon>
#include<QCloseEvent>
#include<QMouseEvent>
#include<QIntValidator>
#include<QDesktopServices>
#include<QTime>
#include<QUrl>

#include<config.h>
#include<msgserver.h>
#include<fileserver.h>
#include<progressui.h>
#include<QLocalSocket>
#include<QLocalServer>
#include<QTextStream>



namespace Ui {
class sharing;
}

class sharing : public QMainWindow
{
    Q_OBJECT

public:
    explicit sharing(QWidget *parent = 0);
    ~sharing();

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void closeEvent(QCloseEvent* event);

public slots:
    void clientChange();
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason);
    void on_SendToPhone();
    void on_SendFile();
    void on_clearConnection();
    void on_restartServer();
    void on_showMainAction();

    void on_exitAppAction();

    void portChange(QString);
    void secretChange(QString);
    void slot_checkBox_ifhidewhenlaunch(bool);
    void slot_checkBox_autoLaunch(bool);
    void showMini();
    void hideWindow();
    void changeFileReceiveLocation();
    void openFileLocationFolder();

    void showProgressUI();
    void setProgressInfo(int fileSize, QString fileName);
    void progressUIChange(qint64);
    void progressUIDestroy();


private:
    Ui::sharing *ui;
    QSystemTrayIcon* mSysTrayIcon;
    QMenu *mMenu;
    QAction *mSendToPhoneAction;
    QAction *mSendFile;
    QAction *mClearConnectionAction;
    QAction *mRestartServiceAction;
    QAction *mShowMainAction;
    QAction *mExitAppAction;
    QIntValidator *valid_port;
    QLocalServer* m_localServer;
    progressUI* progressui;


    msgServer* server;
    fileServer* fileserver;

    QPoint startPoint,endPoint,w_startPoint;  //移动主窗口所用点,分别为鼠标点击起始点。鼠标移动后的点。窗口初始位置
    bool moveFlag;
public:
    void windowInit();
    void setWinFlags();
    void sysTrayIconInit();
    void sysTrayMenuInit();
    void varInit();
    void sysTrayTextChange();



    bool detectSingleInstance();
private:


private slots:
    void newLocalSocketConnection();

};

#endif // SHARING_H
