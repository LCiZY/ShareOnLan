#include<globaldata.h>

//QHash<QString,QString> fileInfo = QHash<QString,QString>();
QQueue<QString> receiveFilesNameQueue = QQueue<QString>();
QQueue<QString> receiveFilesSizeQueue = QQueue<QString>();
QStringList ipList = QStringList();
QString secret="null";
config *conf = new config();
const int FILEBUFFERSIZE = 1024;
int fileSize=0;
int receiveSize=0;
QString sendFileName="untitled.file";







QString getFileInfo(QString filePath){

        QFileInfo info(filePath);
        QString result =QString(" fileName ")+ info.fileName() + QString(" fileSize ")+QString::number(info.size());
        Log(QString("发送文件信息：")+result);
    return result;

}


void Log(QString content){
    qDebug()<<content;
    QFile logFile(conf->configDirectoryPath+QString("/")+conf->logFileName);
    if(logFile.open(QIODevice::Append)){
        QTextStream out(&logFile); out.setCodec("UTF-8");
        out <<QDate::currentDate().toString(Qt::ISODate).toUtf8()<<" "<<QTime::currentTime().toString("hh:mm:ss")<<"\t"<<content.toUtf8()<< "\n" ;
        logFile.close();
    }


}
