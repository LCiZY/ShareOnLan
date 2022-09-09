#ifndef GLOBALDATA_H
#define GLOBALDATA_H
#include<QHash>
#include<QQueue>
#include<QFile>
#include<QFileInfo>
#include<config.h>
#include<QStandardPaths>
#include<QTime>
#include<QDate>
#include<QMutex>
#include<QDebug>
#include <random>
#include <sstream>

namespace uuid {
    std::string generate_uuid_v4();
}

struct SelfFileInfo {
    qint64 fileSize;
    qint64 resolveFileSize;
    QString fileName;
    QString filePath;
    QString uniqueID;
};
typedef SelfFileInfo FileInfo;

struct SelfConnectionInfo {
    quint8 type;
    QString ip;
    quint16 msgPort;
    quint16 fileServerPort;
};
typedef SelfConnectionInfo ConnectionInfo;

extern ConnectionInfo* connectionInfo;
extern QQueue<FileInfo*> receiveFilesQueue;
extern QQueue<FileInfo*> sendFilesQueue;
extern QString FileInfoMsgPreffix;
extern QString FileInfoMsgKey_FileName;
extern QString FileInfoMsgKey_FileSize;
extern QString FileInfoMsgKey_UniqueID;


extern QStringList ipList;
extern QStringList brocastList;
extern QStringList networkcardList;
extern QStringList ipList_Curr;
extern QString secret;
extern config *conf;
extern const int TRANSFERTIMEOUT;
extern const int FILESENDBUFFERSIZE;
extern const int FILEBUFFERSIZE;
extern const int PORT_BOTTOM;
extern const int PORT_TOP;


extern QString FILE_INFO_MSG_HEAD;



class utils {
public:
    static FileInfo* parseFileInfoMsg(QString fileInfoMsg);
    static QString getFileInfoMsg(QString filePath);
    static QFileInfoList GetFileList(QString path);
    static FileInfo* buildFileInfo(QString filePath);
    static QString formatIPSpace(QString ip);
};


class log {
public:
     // 只能传递const char*
     static void info(const char *fmt, ...);
     static void warn(const char *fmt, ...);
     static void error(const char *fmt, ...);
private:

     static void logfunc(QString level, QString content);
};

#endif // GLOBALDATA_H
