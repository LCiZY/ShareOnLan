#include "shareonlan.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    SOLSingleInstanceDetecter d;
    if(d.detectSingleInstance()) return 1; // 利用socket检测是否只有本应用的一个实例
    d.buildLocalServer();

    ShareOnLan w;
    w.setWinFlags();
    w.setLocalServer(d.m_localServer);
    if(!w.listening()) return -2;

    return a.exec();
}
