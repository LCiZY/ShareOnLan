#include<globaldata.h>

QQueue<FileInfo*> receiveFilesQueue = QQueue<FileInfo*>();
QQueue<FileInfo*> sendFilesQueue = QQueue<FileInfo*>();
QString FileInfoMsgPreffix = "${FILEINFO}";
QString FileInfoMsgKey_FileName = "fileName";
QString FileInfoMsgKey_FileSize = "fileSize";
QString FileInfoMsgKey_UniqueID = "uniqueID";

QStringList ipList = QStringList();
QStringList brocastList = QStringList();
QStringList networkcardList = QStringList();
QStringList ipList_Curr = QStringList();
QString secret="null";
config *conf = new config();
const int FILESENDBUFFERSIZE = 40960;
const int FILEBUFFERSIZE = 40960;


QString FILE_INFO_MSG_HEAD="${FILEINFO}";





FileInfo* parseFileInfoMsg(QString fileInfoMsg){
    FileInfo* rt = new FileInfo;
    QStringList splices = fileInfoMsg.split("|");
    for(int i=1;i+1<splices.size();i+=2){
        QString key = splices.at(i), value = splices.at(i+1);
        if(key == FileInfoMsgKey_FileName) rt->fileName = value;
        else if(key == FileInfoMsgKey_FileSize) rt->fileSize = value.toLongLong();
        else if(key == FileInfoMsgKey_UniqueID) rt->uniqueID = value;
    }
    rt->resolveFileSize = 0;
    return rt;
}


QString getFileInfoMsg(QString filePath){

        QFileInfo info(filePath);
        QString result = QString("|%1|").arg(FileInfoMsgKey_FileName) + info.fileName() +
                         QString("|%1|").arg(FileInfoMsgKey_FileSize) + QString::number(info.size()) +
                         QString("|%1|").arg(FileInfoMsgKey_UniqueID) + QString::fromStdString(uuid::generate_uuid_v4());
        Log(QString("发送文件信息：")+result);
    return result;

}

FileInfo* buildFileInfo(QString filePath){
    FileInfo* rt = new FileInfo;
    QFileInfo info(filePath);
    rt->filePath = filePath;
    rt->fileSize = info.size();
    rt->resolveFileSize = 0;
    rt->uniqueID = QString::fromStdString(uuid::generate_uuid_v4());
    rt->fileName = info.baseName();
    return rt;
}



QString formatIPSpace(QString ip){
    QString spaces = "";
    for(int i=0;i<15-ip.size();i++){
        spaces.append("  ");
    }
    return spaces;
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

void LogWithoutTime(QString content){
    qDebug()<<content;
    QFile logFile(conf->configDirectoryPath+QString("/")+conf->logFileName);
    if(logFile.open(QIODevice::Append)){
        QTextStream out(&logFile); out.setCodec("UTF-8");
        out << content.toUtf8() << "\n" ;
        logFile.close();
    }

}




namespace uuid {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::string generate_uuid_v4() {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        };
        return ss.str();
    }
}

