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

#include<QDebug>


//extern QHash<QString,QString> fileInfo;
extern QQueue<QString> receiveFilesNameQueue;
extern QQueue<QString> receiveFilesSizeQueue;
extern QStringList ipList;
extern QStringList networkcardList;
extern QString secret;
extern config *conf;
extern const int FILESENDBUFFERSIZE;
extern const int FILEBUFFERSIZE;
extern int fileSize;
extern int receiveSize;
extern QString sendFileName;

QString getFileInfo(QString filePath);
QString formatIPSpace(QString ip);

void Log(QString content);

#endif // GLOBALDATA_H
