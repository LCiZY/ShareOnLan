#include "shareonlan.h"
#include "ui_shareonlan.h"

ShareOnLan::ShareOnLan(QWidget *parent)
    : DraggableWidget(parent, 0, 30), ui(new Ui::ShareOnLan) {
  ui->setupUi(this);
  log::info("程序开始运行");

  this->setWindowTitle(tr("ShareOnLan"));
  this->setWindowIcon(QIcon(":/icon/transfer_64.png"));
  this->setWindowFlag(Qt::FramelessWindowHint, true);
  this->setAttribute(Qt::WA_TranslucentBackground, true);
  this->setAcceptDrops(true);
  QApplication::setQuitOnLastWindowClosed(false);
  QNetworkProxyFactory::setUseSystemConfiguration(false);

  serverInit();

  windowInit();
  sysTrayIconInit(); // 依赖msgserver的初始化
  sysTrayMenuInit();
  varInit();
}

ShareOnLan::~ShareOnLan() {
  log::info("主窗体析构调用");
  fileserver->deleteLater();
  server->deleteLater();
  setting->deleteLater();
  if (connect2ui != nullptr)
    connect2ui->deleteLater();
  delete ui;
  log::info("主窗体析构调用完成，程序结束运行");
}

bool ShareOnLan::listening() {
  return server->listenOn(server->listeningPort) &&
         fileserver->listenOn(fileserver->fileServerListeningPort);
}

void ShareOnLan::serverInit() {
  server = new msgServer(this, setting->get(UserSetting::Item::PORT).toInt());
  fileserver = new fileServer;
  connect(server, SIGNAL(clientChange()), this, SLOT(clientChange()));
  connect(server, SIGNAL(ipChange()), this, SLOT(ipChange()));
  connect(server, SIGNAL(otherPCReadyReceiveFile()), this,
          SLOT(otherPCReadyReceiveFile()));
  connect(fileserver, SIGNAL(newFileConnection()), this,
          SLOT(showProgressUI()));
  connect(fileserver, SIGNAL(fileTransferDone()), this,
          SLOT(progressUIDestroy()));
  connect(fileserver, SIGNAL(sendNextFile()), this, SLOT(triggerSendFile()));
  connect(fileserver, SIGNAL(currFileInfo(qint64, QString)), this,
          SLOT(setProgressInfo(qint64, QString)));
  connect(fileserver, SIGNAL(receiveProgress(qint64)), this,
          SLOT(progressUIChange(qint64)));
  connect(fileserver, SIGNAL(sendProgress(qint64)), this,
          SLOT(progressUIChange(qint64)));
}

void ShareOnLan::windowInit() {

  ui->right_top_icon_label->setPixmap(QPixmap(":/icon/transfer_3d.png"));
  ui->lineEdit_port->setText(setting->get(UserSetting::Item::PORT));
  valid_port = new QIntValidator(this);
  valid_port->setRange(AppContext::PORT_BOTTOM, AppContext::PORT_TOP);
  ui->lineEdit_port->setValidator(valid_port);
  ui->lineEdit_port->setToolTip(
      QString("%1 <= 端口号 <= %2")
          .arg(QString::number(AppContext::PORT_BOTTOM))
          .arg(QString::number(AppContext::PORT_TOP)));
  ui->checkBox_isHideAtStartup->setChecked(
      setting->get(UserSetting::Item::IS_HIDE_AT_STARTUP).compare("true") == 0);
  ui->checkBox_ifautostartup->setChecked(
      setting->get(UserSetting::Item::AUTOMATIC_STARTUP).compare("true") == 0);
  setting->setAutomaticStartup(ui->checkBox_ifautostartup->isChecked());
  ui->lineEdit_secret->setText(setting->get(UserSetting::Item::SECRET));
  ui->lineEdit_secret->setToolTip("输入手机APP上的密钥");
  ui->lineEdit_fileReceiveLocation->setText(
      setting->get(UserSetting::Item::FILE_RECEIVE_LOCATION));
  ui->lineEdit_fileReceiveLocation->setToolTip(
      ui->lineEdit_fileReceiveLocation->text());
  ui->lineEdit_fileReceiveLocation->setEnabled(false);
  ui->pushButton_changeFileReceiveLocation->setToolTip("更改文件保存位置");
  ui->pushButton_changeFileReceiveLocation->setIcon(
      QIcon(":/images/button_edit_location.png"));
  ui->pushButton_changeFileReceiveLocation->setIconSize(QSize(21, 21));
  ui->pushButton_openFileLocationFolder->setToolTip("打开保存文件的文件夹");
  ui->pushButton_openFileLocationFolder->setIcon(
      QIcon(":/images/button_directory.png"));
  ui->pushButton_openFileLocationFolder->setIconSize(QSize(21, 21));

  QFile styleFile(":/style/style.qss");
  if (styleFile.open(QIODevice::ReadOnly)) {
    QString style = styleFile.readAll();
    this->setStyleSheet(style);
  }

  connect(ui->lineEdit_secret, SIGNAL(textChanged(QString)), this,
          SLOT(secretChange(QString)));
  connect(ui->lineEdit_port, SIGNAL(textChanged(QString)), this,
          SLOT(portChange(QString)));
  connect(ui->checkBox_isHideAtStartup, SIGNAL(toggled(bool)), this,
          SLOT(slot_checkBox_isHideAtStartup(bool)));
  connect(ui->checkBox_ifautostartup, SIGNAL(toggled(bool)), this,
          SLOT(slot_checkBox_autoLaunch(bool)));
  connect(ui->pushButton_changeFileReceiveLocation, SIGNAL(clicked(bool)), this,
          SLOT(changeFileReceiveLocation()));
  connect(ui->pushButton_openFileLocationFolder, SIGNAL(clicked(bool)), this,
          SLOT(openFileLocationFolder()));
  connect(ui->minimalButton, SIGNAL(clicked(bool)), this, SLOT(showMini()));
  connect(ui->closeButton, SIGNAL(clicked(bool)), this, SLOT(hideWindow()));
}

void ShareOnLan::varInit() {
  progressui = nullptr;
  connect2ui = nullptr;
}

void ShareOnLan::setWinFlags() {
  //设置窗口本身是否可见
  this->setVisible(!ui->checkBox_isHideAtStartup->isChecked());
  //设置托盘图标
  this->mSysTrayIcon->setVisible(true);
}

void ShareOnLan::sysTrayIconInit() {

  //创建QSystemTrayIcon对象
  mSysTrayIcon = new QSystemTrayIcon(this);
  //设置QSystemTrayIcon图标
  mSysTrayIcon->setIcon(QIcon(":/icon/transfer.png"));
  //为托盘图标绑定槽函数
  connect(mSysTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this,
          SLOT(on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason)));
}

void ShareOnLan::on_activatedSysTrayIcon(
    QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
  case QSystemTrayIcon::DoubleClick:
    //双击托盘图标
    //双击后显示主程序窗口
    this->on_showSettingAction();

    break;
  case QSystemTrayIcon::Unknown:
  case QSystemTrayIcon::Trigger:
  case QSystemTrayIcon::MiddleClick:
    break;
  case QSystemTrayIcon::Context:
    //右击托盘图标
    QPainterPath path;
    QRectF rect =
        QRectF(0, 0, mMenu->sizeHint().width(), mMenu->sizeHint().height());
    path.addRoundedRect(rect, 4, 4);
    QPolygon polygon = path.toFillPolygon().toPolygon();
    QRegion region(polygon);
    mMenu->setMask(region);
    break;
  }
}

void ShareOnLan::sysTrayMenuInit() {

  mMenu = new QMenu(this);
  // 设置圆角的前置设置
  //        mMenu->setWindowFlags(Qt::FramelessWindowHint);
  //        mMenu->setAttribute(Qt::WA_TranslucentBackground);

  mSendToPhoneAction =
      new QAction(QIcon(":/images/icon_message.png"), tr("发送文本"), mMenu);
  connect(mSendToPhoneAction, SIGNAL(triggered()), this,
          SLOT(on_SendToPhone()));

  mSendFile =
      new QAction(QIcon(":/images/icon_send_file.png"), tr("发送文件"), mMenu);
  connect(mSendFile, SIGNAL(triggered()), this, SLOT(on_SendFile()));

  mConnectInfoAction =
      new QAction(QIcon(":/images/icon_link.png"), tr("连接信息"), mMenu);
  connect(mConnectInfoAction, SIGNAL(triggered()), this,
          SLOT(on_ShowNetInfo()));

  mRestartServiceAction =
      new QAction(QIcon(":/images/icon_reboot.png"), tr("重启服务"), mMenu);
  connect(mRestartServiceAction, SIGNAL(triggered()), this,
          SLOT(on_restartServer()));

  mConnectToAction =
      new QAction(QIcon(":/images/icon_connect_to.png"), tr("连接至"), mMenu);
  connect(mConnectToAction, SIGNAL(triggered()), this, SLOT(on_connectTo()));

  mSettingAction =
      new QAction(QIcon(":/images/icon_setting.png"), tr("设置"), mMenu);
  connect(mSettingAction, SIGNAL(triggered()), this,
          SLOT(on_showSettingAction()));

  mExitAppAction = new QAction(tr("退出"), mMenu);
  connect(mExitAppAction, SIGNAL(triggered()), this, SLOT(on_exitAppAction()));

  //新增菜单项---发送文本
  mMenu->addAction(mSendToPhoneAction);
  //新增菜单项---发送文件
  mMenu->addAction(mSendFile);
  //增加分隔符----------
  mMenu->addSeparator();
  //新增菜单项---关闭连接
  mMenu->addAction(mConnectInfoAction);
  //新增菜单项---重启服务
  mMenu->addAction(mRestartServiceAction);
  //新增菜单项---连接至
  mMenu->addAction(mConnectToAction);
  //新增菜单项---显示主界面
  mMenu->addAction(mSettingAction);
  //增加分隔符----------
  mMenu->addSeparator();
  //新增菜单项---退出程序
  mMenu->addAction(mExitAppAction);
  //把QMenu赋给QSystemTrayIcon对象
  mSysTrayIcon->setContextMenu(mMenu);

  //初始化tooltip
  sysTrayTextChange();
}

//监听的端口号有变更
void ShareOnLan::portChange(QString newPort) {
  if (newPort.isEmpty() || newPort.isNull() || newPort.compare("") == 0 ||
      newPort.toInt() <= 1024)
    return;
  if (server->listenOn(newPort.toInt())) {
    setting->set(UserSetting::Item::PORT, newPort);
    sysTrayTextChange();
  } else {
    QMessageBox::critical(nullptr, QString("绑定端口失败"),
                          QString("端口%1被占用，请更换端口").arg(newPort),
                          QMessageBox::Ok);
    ui->lineEdit_port->setText("");
  }
}

void ShareOnLan::secretChange(QString newSecret) {
  if (newSecret.isNull() || newSecret.compare("") == 0)
    return;
  newSecret = newSecret.simplified();
  newSecret = newSecret.replace(" ", "");
  setting->set(UserSetting::Item::SECRET, newSecret);
}

void ShareOnLan::slot_checkBox_isHideAtStartup(bool checked) {
  setting->set(UserSetting::Item::IS_HIDE_AT_STARTUP,
               checked ? "true" : "false");
}

void ShareOnLan::changeFileReceiveLocation() {
  QString currDir =
      ui->lineEdit_fileReceiveLocation->text().compare("") == 0
          ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
          : ui->lineEdit_fileReceiveLocation->text();
  QString location =
      QFileDialog::getExistingDirectory(this, tr("选择文件保存位置"), currDir);
  if (location.compare("") == 0)
    return;
  ui->lineEdit_fileReceiveLocation->setText(location);
  ui->lineEdit_fileReceiveLocation->setToolTip(location);
  setting->set(UserSetting::Item::FILE_RECEIVE_LOCATION, location);
}

void ShareOnLan::openFileLocationFolder() {
  QString currDir =
      ui->lineEdit_fileReceiveLocation->text().compare("") == 0
          ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)
          : ui->lineEdit_fileReceiveLocation->text();
  if (currDir.compare("") == 0)
    return;
  QDesktopServices::openUrl(QUrl(tr("file:///") + currDir, QUrl::TolerantMode));
}

void ShareOnLan::clientChange() {
  //托盘tooltip改变
  sysTrayTextChange();
  //如果控制连接断开：进度条页面存在则关闭； 关闭filesocket
  if (!server->ifConnected()) {
    if (this->progressui != nullptr) {
      progressUIDestroy();
    }
    fileserver->closeSocket();
    AppContext::sendFilesQueue.clear();
    AppContext::receiveFilesQueue.clear();
  }
}

void ShareOnLan::ipChange() {
  server->getLanBrocastAddress();
  sysTrayTextChange();
}

void ShareOnLan::connectToOtherPC(QString ip, quint16 port) {
  log::info("%s", QString("连接到其他PC， ip:%1  port:%2")
                      .arg(ip)
                      .arg(QString::number(port))
                      .toStdString()
                      .c_str());
  QTcpSocket *socket = new QTcpSocket();
  socket->setProxy(QNetworkProxy::NoProxy);
  socket->connectToHost(ip, port);
  bool ok = socket->waitForConnected(3000);
  QAbstractSocket::SocketState state = socket->state();
  if (ok && state == QAbstractSocket::ConnectedState) {
    server->incomingConnection(socket);
    log::info("连接到其他PC成功");
    connect2ui->close();
  } else {
    QMessageBox::critical(
        nullptr, QString("连接失败"),
        QString("连接至其他PC失败：%1").arg(socket->errorString()),
        QMessageBox::Ok);
    log::error("连接到其他PC失败");
  }
}

void ShareOnLan::otherPCReadyReceiveFile() {
  if (!server->ifConnected())
    return;
  FileSocket *socket = new FileSocket(-1);
  socket->setProxy(QNetworkProxy::NoProxy);
  socket->connectToHost(
      server->getSocket()->peerAddress(),
      fileserver->fileServerListeningPort); //应该写对方的文件服务器端口
  bool ok = socket->waitForConnected(3000);
  QAbstractSocket::SocketState state = socket->state();
  if (ok && state == QAbstractSocket::ConnectedState) {
    fileserver->incomingConnection(socket);
  }
}

void ShareOnLan::sysTrayTextChange() {
  QString content = "";
  for (int i = 0; i < AppContext::ipList.size(); i++) {
    content.append("\n")
        .append("IP:")
        .append(AppContext::ipList.at(i))
        .append(" (")
        .append(AppContext::networkcardList.at(i))
        .append(")");
  }
  tray_tooltip = tr("「ShareOnLan」正在运行")
                     .append("\n连接状态:")
                     .append(server->getConnection())
                     .append(content)
                     .append("\n端口:")
                     .append(QString::number(server->listeningPort));
  //当鼠标移动到托盘上的图标时，会显示此处设置的内容
  mSysTrayIcon->setToolTip(tray_tooltip);
}

void ShareOnLan::on_SendToPhone() {
  if (!this->server->ifConnected()) {
    QMessageBox::about(nullptr, "失败", "未连接至手机");
    return;
  }
  //写入剪贴板
  QClipboard *board = QApplication::clipboard();

  this->server->sendMsg(board->text() + "\n");
}

void ShareOnLan::on_SendFile() {
  if (!this->server->ifConnected()) {
    QMessageBox::about(nullptr, "失败", "未连接至手机");
    return;
  }

  QStringList filePaths;
  QString filePath = "";

  const QClipboard *clipboard = QApplication::clipboard(); //获取剪切版内容
  //为数据提供一个容器，用来记录关于MIME类型数据的信息
  //常用来描述保存在剪切板里信息，或者拖拽原理
  const QMimeData *mimeData = clipboard->mimeData();
  if (mimeData->hasUrls()) { // 如果剪贴板有文件，询问用户是否发送此文件
    QList<QUrl> urls = mimeData->urls();
    bool exist = true;
    for (int i = 0; i < urls.size(); i++) {
      QString urlPath = urls.at(i).toLocalFile();
      QFileInfo file(urlPath);
      if (!file.exists()) {
        exist = false;
        break;
      }
      if (file.isFile()) {
        filePaths.append(urlPath);
      } else {
        QFileInfoList infoList = utils::GetFileList(urlPath);
        for (QFileInfo info : infoList) {
          filePaths.append(info.absoluteFilePath());
        }
      }
    }
    if (!filePaths.isEmpty()) {
      QString dlgTitle = "发送文件";
      QString strInfo = QString("检测到剪贴板中的 %1 等共 %2 个文件，是否发送?")
                            .arg(filePaths.at(0))
                            .arg(QString::number(filePaths.size()));
      int result = QMessageBox::question(nullptr, dlgTitle, strInfo, "是",
                                         "选择其他文件", "取消");
      if (result == 0) { //是
        if (!exist) {
          log::error("%s", "剪贴板中的文件不存在！");
          QMessageBox::critical(nullptr, "错误", "文件不存在");
          return;
        }
      } else if (result == 1) { //选择文件
        filePaths.clear();
      } //取消
      else
        return;
    }
  }

  if (filePaths.isEmpty())
    filePaths = QFileDialog::getOpenFileNames(
        nullptr, "请选择要发送的文件",
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
  if (filePaths.isEmpty())
    return;

  for (QString filePath : filePaths) {
    AppContext::sendFilesQueue.push_back(utils::buildFileInfo(filePath));
  }
  triggerSendFile();
}

/* dropEvent 过滤无用urls，并将有效部分交给文件读取方式加载 */
void ShareOnLan::dropEvent(QDropEvent *event) {
  QList<QUrl> urls = event->mimeData()->urls();
  QStringList filePaths;
  for (int i = 0; i < urls.size(); i++) {
    QString urlPath = urls.at(i).toLocalFile();
    QFileInfo file(urlPath);
    if (file.isFile()) {
      filePaths.append(urlPath);
    } else {
      QFileInfoList infoList = utils::GetFileList(urlPath);
      for (QFileInfo info : infoList) {
        filePaths.append(info.absoluteFilePath());
      }
    }
  }

  if (filePaths.isEmpty())
    return;

  for (QString filePath : filePaths) {
    AppContext::sendFilesQueue.push_back(utils::buildFileInfo(filePath));
  }
  triggerSendFile();
}

void ShareOnLan::triggerSendFile() {
  if (AppContext::sendFilesQueue.isEmpty())
    return;
  this->fileserver->ifSend = true;
  QString filePath = AppContext::sendFilesQueue.at(0)->filePath;
  log::info("%s", QString("发送文件：").append(filePath).toStdString().c_str());
  this->server->sendMsg(AppContext::FileInfoMsgPreffix +
                        utils::getFileInfoMsg(filePath));
}

void ShareOnLan::on_connectTo() {
  if (connect2ui == nullptr) {
    connect2ui = new Connect2UI(setting);
    connect(connect2ui, SIGNAL(confirmConnect(QString, quint16)), this,
            SLOT(connectToOtherPC(QString, quint16)));
  }
  connect2ui->show();
}

void ShareOnLan::on_showSettingAction() {
  this->move(
      (QApplication::desktop()->availableGeometry().width() - rect().width()) /
          2,
      (QApplication::desktop()->availableGeometry().height() -
       rect().height()) /
          2);
  this->showMain();
}

//显示本机IP等信息
void ShareOnLan::on_ShowNetInfo() {
  QMessageBox::about(nullptr, "连接信息", tray_tooltip);
}

//断开所有连接,清空文件接收队列
void ShareOnLan::clearConnection() {
  server->closeSocket();
  fileserver->closeSocket();
  //清空文件接收队列
  AppContext::receiveFilesQueue.clear();
  AppContext::sendFilesQueue.clear();
}

//重启server：重新监听端口并resume连接并重新检测网络信息
void ShareOnLan::on_restartServer() {
  clearConnection();
  server->getLanBrocastAddress();
  server->serverShutDown();
  fileserver->serverShutDown();
  server->listenOn(server->listeningPort);
  server->resumeAccepting();
  fileserver->listenOn(fileserver->fileServerListeningPort);
  fileserver->resumeAccepting();
  sysTrayTextChange();
}

void ShareOnLan::slot_checkBox_autoLaunch(bool flag) {
  setting->set(UserSetting::Item::AUTOMATIC_STARTUP, flag ? "true" : "false");
  setting->setAutomaticStartup(flag);
}

void ShareOnLan::showMain() {
  this->show();
  this->raise();
  this->activateWindow();
}

void ShareOnLan::showMini() { this->showMinimized(); }

void ShareOnLan::hideWindow() { this->hide(); }

void ShareOnLan::on_exitAppAction() {
  this->server->close();
  this->fileserver->close();
  qApp->exit();
}

void ShareOnLan::showProgressUI() {
  if (this->progressui != nullptr) {
    progressui->show();
    return;
  }
  progressui = new ProgressUI(nullptr);
  connect(progressui, SIGNAL(destroyed(QObject *)), this,
          SLOT(progressUIDestroy()));
  progressui->showAtBottomRight();
}

void ShareOnLan::setProgressInfo(qint64 fileSize, QString fileName) {
  if (this->progressui == nullptr)
    return;
  progressui->setCurrTaskInfo(fileSize, fileName);
}

void ShareOnLan::progressUIChange(qint64 pg) {
  if (this->progressui == nullptr)
    return;
  this->progressui->changeUI(pg);
}

void ShareOnLan::progressUIDestroy() {
  if (this->progressui == nullptr)
    return;
  this->progressui->close();
  this->progressui->deleteLater();
  this->progressui = nullptr;
}

/* dragEnterEvent 过滤无用Event */
void ShareOnLan::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();
  }
}

void ShareOnLan::keyPressEvent(QKeyEvent *ev) {
  if (ev->key() == Qt::Key_Escape) {
    close();
  }
}

//改写了关闭事件，先关闭服务器监听循环再接受关闭事件
void ShareOnLan::closeEvent(QCloseEvent *event) {
  /**以下四句为正常关闭的代码*/
  //     this->server->close();
  //     this->fileserver->close();
  //     qApp->exit();
  //     event->accept();
  this->hide();
  event->ignore();
}

void ShareOnLan::setLocalServer(QLocalServer *m_localServer) {
  this->m_localServer = m_localServer;
  connect(m_localServer, SIGNAL(newConnection()), this,
          SLOT(newLocalSocketConnection())); //监听新到来的连接
}

void ShareOnLan::newLocalSocketConnection() {
  QLocalSocket *socket = m_localServer->nextPendingConnection();
  if (!socket)
    return;
  socket->waitForReadyRead(1000);
  socket->deleteLater();
  this->showMain();
}

/* SOLSingleInstanceDetecter 单进程检测类 */

bool SOLSingleInstanceDetecter::detectSingleInstance() {

  QLocalSocket socket; //在Windows上是有名管道，在Linux上是socket
  socket.connectToServer(serverName);
  if (socket.waitForConnected(
          500)) { //如果能够连接得上的话，将参数发送到服务器，然后退出
    return true;
  }

  return false;
}

bool SOLSingleInstanceDetecter::buildLocalServer() {
  //运行到这里，说明没有实例在运行，那么创建服务器。
  m_localServer = new QLocalServer(this);
  if (!m_localServer->listen(serverName)) {
    if (m_localServer->serverError() == QAbstractSocket::AddressInUseError &&
        QFile::exists(m_localServer->serverName())) { //确保能够监听成功
      QFile::remove(m_localServer->serverName());
      return m_localServer->listen(serverName);
    }
  }
  return false;
}
