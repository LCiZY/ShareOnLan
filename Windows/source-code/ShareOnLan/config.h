#ifndef CONFIG_H
#define CONFIG_H
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


//注册表
#define AUTO_RUN "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"


class config:public QObject
{
public:
    config(QObject* parent=0);
    ~config();

    QHash<QString,QString> defaultConfigurations;
    QHash<QString,QString> configurations;

    void configInit();

    void enableDefaultConfig();

    bool writeToConfigFile(QString configurationItem, QString configuration);
    bool readConfigFromConfigFile();
    bool readDefaultConfigFromDefaultConfigFile();
    bool isPortValid();

    QString getConfig(QString configName);
    bool setConfig(QString configurationItem, QString configuration);
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

#endif // CONFIG_H
