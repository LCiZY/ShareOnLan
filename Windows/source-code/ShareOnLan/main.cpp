#include "shareonlan.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ShareOnLan w;
    w.setWinFlags();
    if(w.detectSingleInstance()) return 0;
    return a.exec();
}
