#include "sharing.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    sharing w;
    w.setWinFlags();
    if(w.detectSingleInstance()) return 0;
    return a.exec();
}
