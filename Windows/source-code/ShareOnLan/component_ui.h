#ifndef COMPONENT_UI_H
#define COMPONENT_UI_H

#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QRegExp>
#include <QTimer>
#include <QWidget>
#include <globaldata.h>
#include <user_setting.h>

#include <QDebug>
#include <QMainWindow>

class DraggableWidget : public QWidget {
  Q_OBJECT

public:
  explicit DraggableWidget(QWidget *parent = 0, int x = 0, int y = 0);
  ~DraggableWidget();

protected:
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

private:
  QPoint mousePressPoint, mouseMoveCurrPoint,
      windowInitialPoint; // 移动主窗口所用变量,分别为鼠标点击起始点、鼠标移动后的点、窗口初始位置
  bool isDragging;        // 是否处于拖拽状态
  int rightBorder,
      bottomBorder; // 分别代表鼠标点击可触发移动区域的x和y,鼠标可触发的区域为Point(lefttop)~Point(lefttop)+Point(xy)
};

// ---------------------------------------------------

namespace Ui {
class Connect2UI;
}

class Connect2UI : public QWidget {
  Q_OBJECT

public:
  explicit Connect2UI(UserSetting *c, QWidget *parent = 0);
  ~Connect2UI();

signals:
  void confirmConnect(QString, quint16);

protected:
  virtual void keyPressEvent(QKeyEvent *ev);

private:
  Ui::Connect2UI *ui;
};

// ----------------------------------------------------

namespace Ui {
class ProgressUI;
}

class ProgressUI : public DraggableWidget {
  Q_OBJECT

public:
  explicit ProgressUI(QWidget *parent = 0);
  ~ProgressUI();

  void setCurrTaskInfo(qint64 fileSize, QString fileName);
  void showAtBottomRight();
  void addHeightOnBase(int h);

public slots:
  void changeUI(qint64);
  void onFadeIn();
  void changeUILinearly();

private:
  Ui::ProgressUI *ui;
  QTimer *showTimer;
  QTimer *linearIncreateBarTimer;

  QRect deskRect;
  QPoint normalPoint;

  int transferCount;
  qint64 dlength;
  int widgetHeight, bgHeight, tooltipHeight;
};

#endif // COMPONENT_UI_H
