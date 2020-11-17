#include "progressui.h"
#include "ui_progressui.h"

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




    QFile styleFile(":/style/progressUIStyle.qss");if(styleFile.open(QIODevice::ReadOnly)){QString style=styleFile.readAll(); this->setStyleSheet(style);}
}

progressUI::~progressUI()
{
    delete ui;
    qDebug()<<"Progressui Deconstruction Invoked.";
}


void progressUI::changeUI(qint64 progress){
    transferCount +=(int)progress;
}

void progressUI::changeUILinearly(){
    if(ui->progressBar->value()<transferCount){
        if(transferCount==ui->progressBar->maximum()){ui->progressBar->setValue(transferCount);linearIncreateBarTimer->stop();  }
        else ui->progressBar->setValue(ui->progressBar->value()+dlength);
    }else
    if(transferCount==ui->progressBar->maximum()){ui->progressBar->setValue(transferCount);linearIncreateBarTimer->stop();  }
}

void  progressUI::setCurrTaskInfo(int fileSize,  QString fileName){
    ui->progressBar->setMaximum(fileSize);
    ui->label_tooltip->setText(tr("正在传输文件：")+fileName);
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
