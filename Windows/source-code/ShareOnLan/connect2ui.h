#ifndef CONNECT2UI_H
#define CONNECT2UI_H

#include <QWidget>
#include<config.h>
#include<QRegExp>
#include<globaldata.h>
#include<QMessageBox>

namespace Ui {
class Connect2UI;
}

class Connect2UI : public QWidget
{
    Q_OBJECT

public:
    explicit Connect2UI(config *c, QWidget *parent = 0);
    ~Connect2UI();

signals:
    void confirmConnect(QString, quint16);

private:
    Ui::Connect2UI *ui;
};

#endif // CONNECT2UI_H
