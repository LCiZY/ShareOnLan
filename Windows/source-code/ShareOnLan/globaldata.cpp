#include<globaldata.h>


/* AppContext 应用配置、全局变量 */
UserSetting *setting = new UserSetting();

const int AppContext::TRANSFERTIMEOUT = 5000; // ms
const int AppContext::FILESENDBUFFERSIZE = 40960;
const int AppContext::FILEBUFFERSIZE = 40960;
const int AppContext::PORT_BOTTOM = 1025;
const int AppContext::PORT_TOP = 65533;

const QString AppContext::FileInfoMsgPreffix = "${FILEINFO}";
const QString AppContext::FileInfoMsgKey_FileName = "fileName";
const QString AppContext::FileInfoMsgKey_FileSize = "fileSize";
const QString AppContext::FileInfoMsgKey_UniqueID = "uniqueID";

QQueue<FileInfo*> AppContext::receiveFilesQueue = QQueue<FileInfo*>();
QQueue<FileInfo*> AppContext::sendFilesQueue = QQueue<FileInfo*>();

QStringList AppContext::ipList = QStringList();
QStringList AppContext::brocastList = QStringList();
QStringList AppContext::networkcardList = QStringList();
QStringList AppContext::ipListCurr = QStringList();





/* 工具函数 */
namespace utils {
    FileInfo* parseFileInfoMsg(QString fileInfoMsg){
        FileInfo* rt = new FileInfo;
        QStringList splices = fileInfoMsg.split("|");
        for(int i=1;i+1<splices.size();i+=2){
            QString key = splices.at(i), value = splices.at(i+1);
            if(key == AppContext::FileInfoMsgKey_FileName) rt->fileName = value;
            else if(key == AppContext::FileInfoMsgKey_FileSize) rt->fileSize = value.toLongLong();
            else if(key == AppContext::FileInfoMsgKey_UniqueID) rt->uniqueID = value;
        }
        rt->resolveFileSize = 0;
        return rt;
    }


    QString getFileInfoMsg(QString filePath){

        QFileInfo info(filePath);
        QString result = QString("|%1|").arg(AppContext::FileInfoMsgKey_FileName) + info.fileName() +
                QString("|%1|").arg(AppContext::FileInfoMsgKey_FileSize) + QString::number(info.size()) +
                QString("|%1|").arg(AppContext::FileInfoMsgKey_UniqueID) + QString::fromStdString(uuid::generate_uuid_v4());
        log::info("%s", QString("发送文件信息：%1").arg(result).toStdString().c_str());
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

    QFileInfoList GetFileList(QString path)
    {
        QDir dir(path);
        QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
        QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

        for(int i = 0; i != folder_list.size(); i++)
        {
            QString name = folder_list.at(i).absoluteFilePath();
            QFileInfoList child_file_list =  GetFileList(name);
            file_list.append(child_file_list);
        }

        return file_list;
    }

    QString formatIPSpace(QString ip){
        QString spaces = "";
        for(int i=0;i<15-ip.size();i++){
            spaces.append("  ");
        }
        return spaces;
    }
}

/* 日志 */
void log::info(const char *fmt, ...){
    va_list list;
    va_start(list, fmt);
    QString str = QString().vsprintf(fmt, list);
    va_end(list);
    log::logfunc("info", str);
}

void log::warn(const char *fmt, ...){
    va_list list;
    va_start(list, fmt);
    QString str = QString().vsprintf(fmt, list);
    va_end(list);
    log::logfunc("warn", str);
}

void log::error(const char *fmt, ...){
    va_list list;
    va_start(list, fmt);
    QString str = QString().vsprintf(fmt, list);
    va_end(list);
    log::logfunc("error", str);
}
QMutex logMutex;
void log::logfunc(QString level, QString content){
    QString nowDate = QString(QDate::currentDate().toString(Qt::ISODate).toUtf8());
    QString nowTime = QTime::currentTime().toString("hh:mm:ss");
    QFile logFile(setting->configDirectoryPath+QString("/")+setting->logFileName);
    logMutex.lock();
    QString logStr = QString("%1 %2 %3\t%4").arg(level).arg(nowDate).arg(nowTime).arg(content);
    qDebug(logStr.toStdString().c_str());
    if(logFile.open(QIODevice::Append)){
        QTextStream out(&logFile); out.setCodec("UTF-8");
        out << logStr << "\n" ;
        logFile.close();
    }
    logMutex.unlock();
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

