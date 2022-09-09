#ifndef GLOBALDATA_H
#define GLOBALDATA_H
#include<QHash>
#include<QQueue>
#include<QFile>
#include<QFileInfo>
#include<user_setting.h>
#include<QStandardPaths>
#include<QTime>
#include<QDate>
#include<QMutex>
#include<QDebug>
#include <random>
#include <sstream>

struct SelfFileInfo {
    qint64 fileSize;
    qint64 resolveFileSize;
    QString fileName;
    QString filePath;
    QString uniqueID;
};
typedef SelfFileInfo FileInfo;

extern UserSetting *setting;

class AppContext {
public:
    // 接收文件的最长无输入等待时间，ms
    static const int TRANSFERTIMEOUT;
    // 发送文件缓冲区大小，字节
    static const int FILESENDBUFFERSIZE;
    // 接收文件缓冲区大小，字节
    static const int FILEBUFFERSIZE;
    // 消息服务器端口号下界
    static const int PORT_BOTTOM;
    // 消息服务器端口号上界
    static const int PORT_TOP;

    /*-----收发文件元信息-----*/
    // 文件信息的header
    static const QString FileInfoMsgPreffix;
    // 文件信息中文件名的key
    static const QString FileInfoMsgKey_FileName;
    // 文件信息中文件Size的key
    static const QString FileInfoMsgKey_FileSize;
    // 文件信息中文件唯一ID的key
    static const QString FileInfoMsgKey_UniqueID;

    /*-----网卡信息-----*/
    // 网卡ip列表
    static QStringList ipList;
    // 检测过程中的网卡ip列表
    static QStringList ipListCurr;
    // 广播地址列表
    static QStringList brocastList;
    // 网卡名称列表
    static QStringList networkcardList;

    /*-----收发文件队列-----*/
    // 接收文件队列
    static QQueue<FileInfo*> receiveFilesQueue;
    // 发送文件队列
    static QQueue<FileInfo*> sendFilesQueue;

private:
    AppContext();
};

namespace utils {
    FileInfo* parseFileInfoMsg(QString fileInfoMsg);
    QString getFileInfoMsg(QString filePath);
    QFileInfoList GetFileList(QString path);
    FileInfo* buildFileInfo(QString filePath);
    QString formatIPSpace(QString ip);
}

// 想将log作为namespace，但是好像和sdk里的某个东西重名了。。。 error: redeclared as different kind of symbol
class log {
public:
    // 只能传递const char*
    static void info(const char *fmt, ...);
    static void warn(const char *fmt, ...);
    static void error(const char *fmt, ...);
private:

    static void logfunc(QString level, QString content);
};


namespace uuid {
    std::string generate_uuid_v4();
}


#endif // GLOBALDATA_H
