#ifndef PROGRESSUI_H
#define PROGRESSUI_H

#include <QWidget>
#include<QMouseEvent>
#include<QTimer>
#include<QDesktopWidget>
#include<globaldata.h>

#include<QDebug>

namespace Ui {
class progressUI;
}

class progressUI : public QWidget
{
    Q_OBJECT

public:
    explicit progressUI(QWidget *parent = 0);
    ~progressUI();

    void setCurrTaskInfo(int fileSize, QString fileName);
    void showAtBottomRight();

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
private:
    QPoint startPoint,endPoint,w_startPoint;  //移动主窗口所用点,分别为鼠标点击起始点。鼠标移动后的点。窗口初始位置
    bool moveFlag;

public slots:
    void changeUI(qint64);
    void onFadeIn();
    void changeUILinearly();

private:
    Ui::progressUI *ui;
    QTimer *showTimer;
    QTimer *linearIncreateBarTimer;

    QRect deskRect;
    QPoint normalPoint;

    int transferCount;
    int dlength;


};

#endif // PROGRESSUI_H
