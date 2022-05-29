#include "connect2ui.h"
#include "ui_connect2ui.h"

Connect2UI::Connect2UI(config* c, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Connect2UI)
{
    ui->setupUi(this);
//    this->setWindowFlag(Qt::FramelessWindowHint,true);
//    this->setAttribute(Qt::WA_TranslucentBackground,true);
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
                Log(QString("发起连接至：ip: %1, port:%2").arg(ip).arg(QString::number(port)));
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
