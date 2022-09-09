#include "component_ui.h"
#include "ui_connect2ui.h"
#include "ui_progressui.h"
#include "math.h"


// -------------------------------DraggableWidget  可拖拽的窗口-----------------------------------

DraggableWidget::DraggableWidget(QWidget *parent, int x, int y) : QWidget(parent)
{
    isDragging = false;
    this->rightBorder = x;
    this->bottomBorder = y;
}

DraggableWidget::~DraggableWidget(){
}


void DraggableWidget::mouseMoveEvent(QMouseEvent *event){
    if(isDragging) {
        mouseMoveCurrPoint = event->globalPos() - mousePressPoint + windowInitialPoint;
        this->move(mouseMoveCurrPoint);
    }
}

void DraggableWidget::mousePressEvent(QMouseEvent *event){
    bool isWithinRightBorder = event->globalX() < this->frameGeometry().bottomRight().x();
    if(rightBorder > 0)
        isWithinRightBorder = event->globalX() < this->frameGeometry().topLeft().x() + rightBorder;

    bool isWithinBottomBorder = event->globalY() < this->frameGeometry().bottomRight().y();
    if(bottomBorder > 0)
        isWithinBottomBorder = event->globalY() < this->frameGeometry().topLeft().y() + bottomBorder;

    if( event->globalX() > this->frameGeometry().topLeft().x() && isWithinRightBorder &&
        event->globalY() > this->frameGeometry().topLeft().y() && isWithinBottomBorder ){
         mousePressPoint = event->globalPos();
         windowInitialPoint= this->frameGeometry().topLeft();
         isDragging = true;
    }

}

void DraggableWidget::mouseReleaseEvent(QMouseEvent *){
    isDragging = false;
}


// -------------------------------Connect2UI  连接到其他PC对话框-----------------------------------


Connect2UI::Connect2UI(config* c, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Connect2UI)
{
    ui->setupUi(this);

    this->setWindowTitle(QString("连接至其他PC"));

    connect(ui->connectButton, &QPushButton::clicked,[=](){
            ui->connectButton->setText(QString("正在连接..."));
            ui->connectButton->setEnabled(false);
            QApplication::setOverrideCursor(Qt::WaitCursor);
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

            QString ip = ui->lineEdit_ip->text().trimmed();
            QString p  = ui->lineEdit_port->text().trimmed();
            c->setConfig("otherPCIP", ip);
            c->setConfig("otherPCPort", p);
            quint16 port = (quint16)p.toShort();
            if(PORT_BOTTOM <= port && port <= PORT_TOP && ip.split(".").size() == 4){
                log::info("%s",QString("发起连接至：ip: %1, port:%2").arg(ip).arg(QString::number(port)).toStdString().c_str());
                emit confirmConnect(ip, port);
            }else{
                QMessageBox::critical(nullptr,QString("连接失败"),QString("IP或端口号不合法"),QMessageBox::Ok);
            }

            ui->connectButton->setText(QString("连接"));
            ui->connectButton->setEnabled(true);
            QApplication::restoreOverrideCursor();
       });
    ui->connectButton->setFocus();    //设置默认焦点
    ui->connectButton->setShortcut( QKeySequence::InsertParagraphSeparator );  //设置快捷键为键盘的“回车”键
    ui->connectButton->setShortcut(Qt::Key_Enter);  //设置快捷键为enter键
    ui->connectButton->setShortcut(Qt::Key_Return); //设置快捷键为小键盘上的enter键


    connect(ui->closeButton,&QPushButton::clicked,[=](){
        close();
    });

    ui->lineEdit_ip->setText(c->getConfig("otherPCIP"));
    QRegExp regIp("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    ui->lineEdit_ip->setValidator(new QRegExpValidator(regIp,this));

    ui->lineEdit_port->setText(c->getConfig("otherPCPort"));
    QIntValidator* valid_port=new QIntValidator(this); valid_port->setRange(PORT_BOTTOM,PORT_TOP);
    ui->lineEdit_port->setValidator(valid_port);

    connect(ui->lineEdit_ip, SIGNAL(returnPressed()), ui->connectButton, SIGNAL(clicked()), Qt::UniqueConnection);
    connect(ui->lineEdit_port, SIGNAL(returnPressed()), ui->connectButton, SIGNAL(clicked()), Qt::UniqueConnection);


    ui->label_bg->setStyleSheet("background-color: rgba(255, 255, 255, 255);");

    QWidget::setTabOrder(ui->lineEdit_ip, ui->lineEdit_port);
    QWidget::setTabOrder(ui->lineEdit_port, ui->connectButton);
    QWidget::setTabOrder(ui->connectButton, ui->lineEdit_ip);
}

Connect2UI::~Connect2UI(){
    delete ui;
}
void Connect2UI::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape)
    {
      close();
    }

    QWidget::keyPressEvent(ev);
}

// -------------------------------ProgressUI  文件传输进度窗口-----------------------------------

ProgressUI::ProgressUI(QWidget *parent) :
    DraggableWidget(parent),
    ui(new Ui::ProgressUI)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint,true);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    this->setWindowTitle("文件传输进度");

    showTimer = new QTimer(this);
    connect(showTimer,SIGNAL(timeout()),this,SLOT(onFadeIn()));

    linearIncreateBarTimer = new QTimer(this);
    connect(linearIncreateBarTimer,SIGNAL(timeout()),this,SLOT(changeUILinearly()));

    connect(ui->pushButton_hide,SIGNAL(clicked(bool)),this,SLOT(close()));
    connect(ui->pushButton_hide,SIGNAL(clicked(bool)),this,SLOT(deleteLater()));

    widgetHeight = geometry().height() ;
    bgHeight = ui->label->geometry().height();
    tooltipHeight = ui->label_tooltip->geometry().height();


    QFile styleFile(":/style/progressUIStyle.qss");if(styleFile.open(QIODevice::ReadOnly)){QString style=styleFile.readAll(); this->setStyleSheet(style);}
}

ProgressUI::~ProgressUI()
{
    delete ui;
    log::info("%s", "ProgressUI Deconstruction Invoked.");
}


void ProgressUI::changeUI(qint64 progress){
    transferCount +=(int)progress;
}

void ProgressUI::changeUILinearly(){
    if(ui->progressBar->value()<transferCount){
        if(transferCount>=ui->progressBar->maximum()){ui->progressBar->setValue(ui->progressBar->maximum());linearIncreateBarTimer->stop();  }
        else ui->progressBar->setValue(ui->progressBar->value()+dlength);
    }else
    if(transferCount>=ui->progressBar->maximum()){ui->progressBar->setValue(ui->progressBar->maximum());linearIncreateBarTimer->stop();  }
}

void ProgressUI::addHeightOnBase(int h){
   QRect r1 = geometry();
   r1.setHeight(widgetHeight + h);
   setGeometry(r1);
   QRect r2 =  ui->label->geometry();
   r2.setHeight(bgHeight + h);
   ui->label->setGeometry(r2);
   QRect r3 = ui->label_tooltip->geometry();
   r3.setHeight(tooltipHeight + h);
   ui->label_tooltip->setGeometry(r3);
}

void  ProgressUI::setCurrTaskInfo(qint64 fileSize,  QString fileName){

    ui->progressBar->setMaximum(fileSize);
    QString text = tr("正在传输文件：")+fileName;

    QFontMetrics fm = ui->label_tooltip->fontMetrics();
    int textHeight = fm.height() + 3;
    int widgetWidth = ui->label_tooltip->width();

    int AntoIndex = 1;
    if (!text.isEmpty())
    {
        for (int i = 1; i < text.size() + 1; i++)///
        {
            if (fm.width(text.left(i)) > widgetWidth * AntoIndex)///////当strText宽度大于控件宽度的时候添加换行符
            {
                AntoIndex++;
                text.insert(i - 1, "\n");
            }
            if (fm.width(text.left(i)) > 3 * widgetWidth)////换行超过三行时在末尾添加...省略后面的内容
            {
                text.insert(i - 3, "...\n");
                break;
            }
        }
    }

    if(AntoIndex > 1)
        addHeightOnBase(textHeight * (AntoIndex-1));
    ui->label_tooltip->setText(text);
    ui->label_tooltip->setToolTip(fileName);

    ui->progressBar->setValue(0);
    dlength = fileSize/100;
    transferCount = 0;
    linearIncreateBarTimer->start(20);
}



int dy=0;
void  ProgressUI::showAtBottomRight(){
    QDesktopWidget *deskTop=QApplication::desktop();
        deskRect=deskTop->availableGeometry();
        normalPoint.setX(deskRect.width()-rect().width()-1);
        normalPoint.setY(deskRect.height()-rect().height());
        move(normalPoint.x(),deskRect.height()-1);
        this->setWindowFlag(Qt::WindowStaysOnTopHint);
        this->show();
        this->raise();
        this->activateWindow();

        showTimer->start(10);
        dy=0;
}

// 从右下角滑出
void ProgressUI::onFadeIn(){
    dy+=2;
    move(normalPoint.x(),deskRect.height()-1-dy);

    if(dy>=rect().height()){
        showTimer->stop();
    }

}

