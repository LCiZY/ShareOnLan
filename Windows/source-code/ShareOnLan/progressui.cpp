#include "progressui.h"
#include "ui_progressui.h"
#include "math.h"

progressUI::progressUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::progressUI)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint,true);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    this->setWindowTitle("文件传输进度");


    moveFlag = false;

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

progressUI::~progressUI()
{
    delete ui;
    Log("Progressui Deconstruction Invoked.");
}


void progressUI::changeUI(qint64 progress){
    transferCount +=(int)progress;
}

void progressUI::changeUILinearly(){
    if(ui->progressBar->value()<transferCount){
        if(transferCount>=ui->progressBar->maximum()){ui->progressBar->setValue(ui->progressBar->maximum());linearIncreateBarTimer->stop();  }
        else ui->progressBar->setValue(ui->progressBar->value()+dlength);
    }else
    if(transferCount>=ui->progressBar->maximum()){ui->progressBar->setValue(ui->progressBar->maximum());linearIncreateBarTimer->stop();  }
}

void progressUI::addHeightOnBase(int h){
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

void  progressUI::setCurrTaskInfo(qint64 fileSize,  QString fileName){

    ui->progressBar->setMaximum(fileSize);
    QString text = tr("正在传输文件：")+fileName;

    QFontMetrics fm = ui->label_tooltip->fontMetrics();
//    int textWidth = fm.width(text);
    int textHeight = fm.height() + 3;
    int widgetWidth = ui->label_tooltip->width();
//    int lines = (int)ceil(textWidth*1.0/widgetWidth);

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

//    Log(QString("textWidth:") + QString::number(textWidth)+ QString("  textHeight:") + QString::number(textHeight)  + QString("   lines:") + QString::number(lines) + QString("   AntoIndex:") + QString::number(AntoIndex));
//    Log(QString("text:") + text);
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
void  progressUI::showAtBottomRight(){
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


void progressUI::onFadeIn(){
    dy++;
    move(normalPoint.x(),deskRect.height()-1-dy);


    if(dy>=rect().height()){
        showTimer->stop();
    }

}
























void progressUI::mouseMoveEvent(QMouseEvent *event)
{
    if(moveFlag)
    {
     endPoint= event->globalPos() - startPoint + w_startPoint;
     this->move(endPoint);

    }

}

void progressUI::mousePressEvent(QMouseEvent *event){

    if(event->globalX()>this->frameGeometry().topLeft().x() && event->globalX()<this->frameGeometry().topRight().x() && event->globalY()>this->frameGeometry().topLeft().y() && event->globalY()<this->frameGeometry().bottomRight().y())
   {
         startPoint = event->globalPos();
         w_startPoint= this->frameGeometry().topLeft();
         moveFlag=true;
    }

}

void progressUI::mouseReleaseEvent(QMouseEvent *)
{
    moveFlag=false;

}
