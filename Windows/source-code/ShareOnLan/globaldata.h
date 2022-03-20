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

extern QQueue<FileInfo*> receiveFilesQueue;
extern QQueue<FileInfo*> sendFilesQueue;
extern QString FileInfoMsgPreffix;
extern QString FileInfoMsgKey_FileName;
extern QString FileInfoMsgKey_FileSize;
extern QString FileInfoMsgKey_UniqueID;
extern QMutex sendFileListMutex;


extern QStringList ipList;
extern QStringList brocastList;
extern QStringList networkcardList;
extern QStringList ipList_Curr;
extern QString secret;
extern config *conf;
extern const int FILESENDBUFFERSIZE;
extern const int FILEBUFFERSIZE;


extern QString FILE_INFO_MSG_HEAD;

FileInfo* parseFileInfoMsg(QString fileInfoMsg);
QString getFileInfoMsg(QString filePath);
FileInfo* buildFileInfo(QString filePath);
QString formatIPSpace(QString ip);

void Log(QString content);
void LogWithoutTime(QString content);

#endif // GLOBALDATA_H
