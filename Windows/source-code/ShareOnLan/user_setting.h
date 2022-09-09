#ifndef USER_SETTING_H
#define USER_SETTING_H
#include<QStandardPaths>
#include<QDir>
#include<QSettings>
#include<QString>
#include<QRegExp>
#include<QTextStream>
#include<QHash>
#include<QApplication>

#include <winsock2.h>
#include <Ws2tcpip.h>


// 开机自启注册表的key
#define AUTO_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

enum class UserSettingItem{

};

class UserSetting : public QObject
{
public:
    enum Item
    {
        PORT, // 消息服务器端口号
        AUTOMATIC_STARTUP, // 程序是否开机自启
        IS_HIDE_AT_STARTUP, // 程序启动时是否隐藏主窗口
        SECRET, // 连接密钥
        FILE_RECEIVE_LOCATION, // 文件接收文件夹
        FILE_SERVER_PORT, // 文件服务器端口
        OTHER_PC_IP, // 连接至其他PC - 其他PC的IP
        OTHER_PC_PORT,// 连接至其他PC - 其他PC的消息服务器端口
    };

public:
    UserSetting();
    ~UserSetting();

    QHash<QString,QString> defaultConfigurations;
    QHash<QString,QString> configurations;

    void configInit();

    void enableDefaultConfig();

    bool writeToConfigFile(QString configurationItem, QString configuration);
    bool readConfigFromConfigFile();
    bool readDefaultConfigFromDefaultConfigFile();
    bool isPortValid();

    QString getItemKey(Item item);
    QString get(Item item);
    bool set(Item item, QString configuration);
    void setAutomaticStartup(bool isStart);

public:
    QString configDirectoryPath;
    QString configFilePath;
    QString logFilePath;
    const QString configurationDirectory = "SOLConfig";
    const QString configurationFileName = "SOLAppConfigs.ini";
    const QString logFileName = "SOLLog.log";
    QString DocumentsLocation;

};

#endif // USER_SETTING_H
