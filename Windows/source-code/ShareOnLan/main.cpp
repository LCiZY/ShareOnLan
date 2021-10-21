#include "shareonlan.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SOLSingleInstanceDetecter d;
    if(d.detectSingleInstance()){// 利用socket检测是否只有本应用的一个实例
        Log("已有相同应用正在运行，本程序退出");
        return 1;
    }
    d.buildLocalServer();

    ShareOnLan w;
    w.setWinFlags();
    w.setLocalServer(d.m_localServer);
    if(!w.listening()) {
        Log("启动时监听端口失败，程序退出");
        system((conf->configDirectoryPath+QString("/")+conf->logFileName).toStdString().c_str());
        return -2;
    }

    return a.exec();
}
