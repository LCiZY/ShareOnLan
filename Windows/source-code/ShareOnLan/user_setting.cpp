#include "user_setting.h"

#include <QDebug>

UserSetting::UserSetting() : QObject(nullptr) {
  //获取“我的文档”路径
  DocumentsLocation =
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  configDirectoryPath =
      QString(DocumentsLocation + QString("/") + configurationDirectory);
  configFilePath = configDirectoryPath + QString("/") + configurationFileName;
  logFilePath = configDirectoryPath + QString("/") + logFileName;

  QDir d;
  d.setPath(configDirectoryPath);
  //判断配置目录是否存在(是否第一次使用)，不存在则创建
  if (!d.exists()) {
    d.mkpath(configDirectoryPath);
    configInit();
  }

  //读取默认配置到defaultConfiguration()
  readDefaultConfigFromDefaultConfigFile();

  //判断配置文件是否存在
  QFile confFile(configFilePath);
  if (!confFile.exists()) {
    //配置文件不存在，使用默认配置
    configInit();
    enableDefaultConfig();
  } else {
    //配置文件存在
    //如果读取配置出错，那么使用默认配置，否则使用配置文件的配置
    if (!readConfigFromConfigFile())
      enableDefaultConfig();
  }

  //判断日志文件是否存在
  QFile logFile(logFilePath);
  if (!logFile.exists()) {
    if (logFile.open(QIODevice::WriteOnly)) {
      QTextStream out(&logFile);
      out.setCodec("UTF-8");
      out << QString("#日志\n");
      logFile.close();
    }
  }

  // check配置
  //如果端口无效，使用默认端口
  if (!isPortValid()) {
    set(UserSetting::Item::PORT,
        defaultConfigurations[getItemKey(UserSetting::Item::PORT)]);
  }
}

UserSetting::~UserSetting() {}

void UserSetting::configInit() {
  //读取默认配置文件内容
  QFile defaultConfigFile(":/config/defaultConfiguration.ini");
  QString configs;
  if (defaultConfigFile.open(QIODevice::ReadOnly)) {
    configs = defaultConfigFile.readAll();
    defaultConfigFile.close();
  } else {
    qDebug() << "打开失败:/config/defaultConfiguration.ini";
    return;
  }

  //将默认配置文件内容复制到用户配置文件中
  QFile configFile(configFilePath);
  if (configFile.open(QIODevice::WriteOnly)) {
    QTextStream out(&configFile);
    out.setCodec("UTF-8");
    out << configs.toUtf8() << "\n";
    configFile.close();
  }
  //将不在配置文件中的默认配置（动态获取的默认配置保存至配置文件中）
  set(UserSetting::Item::FILE_RECEIVE_LOCATION,
      defaultConfigurations[getItemKey(
          UserSetting::Item::FILE_RECEIVE_LOCATION)]);
}

QString UserSetting::getItemKey(Item item) {
  switch (item) {
  case Item::PORT:
    return tr("port");
  case Item::AUTOMATIC_STARTUP:
    return tr("automaticStartup");
  case Item::IS_HIDE_AT_STARTUP:
    return tr("isHideAtStartup");
  case Item::SECRET:
    return tr("secret");
  case Item::FILE_RECEIVE_LOCATION:
    return tr("fileReceiveLocation");
  case Item::FILE_SERVER_PORT:
    return tr("fileServerPort");
  case Item::OTHER_PC_IP:
    return tr("otherPCIP");
  case Item::OTHER_PC_PORT:
    return tr("otherPCPort");
  default:
    return tr("");
  }
}

QString UserSetting::get(Item item) {
  QString key = getItemKey(item);
  return configurations[key];
}

bool UserSetting::set(Item item, QString configuration) {
  QString key = getItemKey(item);
  configurations[key] = configuration;
  return writeToConfigFile(key, configuration);
}

bool UserSetting::readConfigFromConfigFile() {

  QFile configFile(configFilePath);
  if (configFile.open(QIODevice::ReadOnly)) {
    char buf[1024];
    qint64 lineLength = 0;
    while (lineLength != -1) {
      lineLength = configFile.readLine(buf, sizeof(buf));
      if (lineLength == -1)
        break;
      QString linedata(buf);

      //去除行两端的空白字符
      linedata = linedata.trimmed();
      //如果是空白行，跳过
      if (linedata.compare("") == 0)
        continue;
      //如果是注释，跳过
      if (linedata.indexOf("#") == 0)
        continue;
      //以“=”号分割行，得到配置名和参数
      QStringList splits = linedata.split("=");
      //去除配置名和参数两边的空白字符
      for (int i = 0; i < splits.size(); i++) {
        splits[i] = splits[i].trimmed();
      }

      if (splits.size() ==
          2) { //如果配置正确，那么splits的大小应该为2,使用配置文件里的配置
        configurations[splits.at(0)] = splits.at(1);
        qDebug("用户配置 %s: %s", splits.at(0).toStdString().c_str(),
               splits.at(1).toStdString().c_str());
      } else if (splits.size() > 0) { //配置错误，使用默认配置
        configurations[splits.at(0)] = defaultConfigurations[splits.at(0)];
      }
    }
    configFile.close();
    return true;
  }

  return false;
}

bool UserSetting::writeToConfigFile(QString configurationItem,
                                    QString configuration) {
  //
  QFile configFile(configFilePath);
  if (!configFile.open(QIODevice::ReadOnly))
    return false;
  QString configs = configFile.readAll();
  configFile.close();
  QRegExp re(configurationItem + "=[^\n]*");
  if (configs.indexOf(re) == -1) { //此项配置不存在
    if (!configFile.open(QIODevice::Append))
      return false;
    QTextStream out(&configFile);
    out.setCodec("UTF-8");
    out << QString(configurationItem + "=" + configuration).toUtf8() << "\n";
    configFile.close();
  } else {
    if (!configFile.open(QIODevice::WriteOnly))
      return false;
    configs = configs.replace(re, configurationItem + "=" + configuration);
    QTextStream out(&configFile);
    out.setCodec("UTF-8");
    out << configs.toUtf8();
    configFile.close();
  }

  return true;
}

bool UserSetting::isPortValid() {
  QString pt = get(UserSetting::Item::PORT);
  if (pt.isEmpty() || pt.isNull())
    return false;
  bool ok = true;
  int portnum = pt.toInt(&ok, 10);
  return ok && portnum > 1024 && portnum < 65535;
}

/*
 *
 *@brief 恢复默认设置
 *
 */
void UserSetting::enableDefaultConfig() {

  QList<QString> keys = defaultConfigurations.keys();
  for (int i = 0; i < keys.length(); i++)
    configurations[keys.at(i)] = defaultConfigurations[keys.at(i)];
}

bool UserSetting::readDefaultConfigFromDefaultConfigFile() {

  QFile configFile(":/config/defaultConfiguration.ini");
  if (configFile.open(QIODevice::ReadOnly)) {
    char buf[128];
    qint64 lineLength = 0;
    //读取defaultConfiguration.ini中的配置
    while (lineLength != -1) {
      lineLength = configFile.readLine(buf, sizeof(buf));
      if (lineLength == -1)
        break;
      QString linedata(buf);

      //去除行两端的空白字符
      linedata = linedata.trimmed();
      //如果是空白行，跳过
      if (linedata.compare("") == 0)
        continue;
      //如果是注释，跳过
      if (linedata.indexOf("#") == 0)
        continue;
      //以“=”号分割行，得到配置名和参数
      QStringList splits = linedata.split("=");
      //去除配置名和参数两边的空白字符
      for (int i = 0; i < splits.size(); i++)
        splits[i] = splits[i].trimmed();

      if (splits.size() ==
          2) { //如果配置正确，那么splits的大小应该为2,使用配置文件里的配置
        defaultConfigurations[splits.at(0)] = splits.at(1);
      } else if (splits.size() > 0) { //配置错误，使用默认配置
        defaultConfigurations[splits.at(0)] = QString();
      }
    }
    //文件默认存放位置：动态获取。
    defaultConfigurations[getItemKey(
        UserSetting::Item::FILE_RECEIVE_LOCATION)] =
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);

    configFile.close();
    return true;
  }

  return false;
}

void UserSetting::setAutomaticStartup(bool isStart) {
  QString strApplicationName = QApplication::applicationName(); //获取应用名称
  QSettings *settings = new QSettings(AUTO_RUN, QSettings::NativeFormat);
  if (isStart) {
    QString strApplicationFilePath =
        QApplication::applicationFilePath(); //获取应用的目录
    settings->setValue(strApplicationName,
                       strApplicationFilePath.replace("/", "\\")); //写入注册表
  } else
    settings->remove(strApplicationName); //移除注册表
  settings->deleteLater();
}
