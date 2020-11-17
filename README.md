# ShareOnLan简介

Windows Connect With Android  
Windows平台与安卓在局域网下互传、发送文本、文件的小工具  
包括Windows端的exe和Android端的app
***

## 开发平台与工具

Windows端是基于QT开发的一个GUI程序

Android端是基于Android studio开发的app
***

## 运行原理&设计思路

![原理图](https://github.com/LCiZY/ShareOnLan/blob/main/images/prototype.png)
***

## 运行流程

**正常流程：**  

1. 由Windows端向所在局域网(LAN)广播UDP包，UDP payload包含PC端的IP、端口号、链接密钥（经过简单加密）。  
2. Android端监听UDP广播包，然后解密payload，验证密钥是否正确：若密钥正确，将IP和端口号填在app的对应输入框里，否则不予置理。  
3. Android的监测线程会每隔数秒自动尝试连接由IP输入框和端口号输入框指定的主机。  
4. 连接成功后，监测线程停止，UDP监听线程停止，PC端会每隔一段时间向Android端发送一条"连接控制消息"（向Android端表明PC端还活着）。  
5. 若Android端在一个超时时间周期内没有收到PC端发来的"连接控制消息"，则认为PC端已离线，就会更新界面（显示为未连接），监测线程会重新开始运行；UDP监听线程也会重新开始监听。  

**一般流程(Android端无法接收Windows端发送的UDP报文)：**

1. 查看PC端程序的托盘Tooltip(提示消息)上的IP和端口号
2. 用户输入PC端的IP和端口
3. Android的监测线程会每隔数秒自动尝试连接由IP输入框和端口号输入框指定的主机。
4. 连接成功后，监测线程停止，UDP监听线程停止，PC端会每隔一段时间向Android端发送一条"连接控制消息"（向Android端表明PC端还活着）。
5. 若Android端在一个超时时间周期内没有收到PC端发来的"连接控制消息"，则认为PC端已离线，就会更新界面（显示为未连接），监测线程会重新开始运行；UDP监听线程也会重新开始监听。

## 运行实例

![PC端主界面](https://github.com/LCiZY/ShareOnLan/blob/main/images/prototype.png)

![PC端托盘图标及ToolTip](https://github.com/LCiZY/ShareOnLan/blob/main/images/systray-tooltip.png)

![PC端更改文件储存位置](https://github.com/LCiZY/ShareOnLan/blob/main/images/changeFileLocation.png)

![PC端打开文件储存文件夹](https://github.com/LCiZY/ShareOnLan/blob/main/images/openFileLocateDir.png)

![Android端未连接/正在连接界面](https://github.com/LCiZY/ShareOnLan/blob/main/images/unconnected-connecting.png)

![Android端已连接界面](https://github.com/LCiZY/ShareOnLan/blob/main/images/connected.png)

![Android端接收到的文件](https://github.com/LCiZY/ShareOnLan/blob/main/images/filesView.png)

<iframe height=500 width=500 src="https://github.com/LCiZY/ShareOnLan/blob/main/images/operateDemo/openReceivedFile.gif"></iframe>  

<iframe height=500 width=500 src="https://github.com/LCiZY/ShareOnLan/blob/main/images/operateDemo/sendFileToPC.gif"></iframe>  

## Bug

1. 长时间运行后Windows端假死
2. Android端发送文件无提示  
