package com.sol.net;


import android.net.Uri;

import java.util.HashSet;
import java.util.LinkedList;
import java.util.concurrent.CopyOnWriteArrayList;

public class ConnectionInfo {

    public static final String RESPONSE = "R";  //PC端接收到消息时返回R, 心跳包
    public static final String FILEINFOCONTROLMESSAGE = "${FILEINFO}";   //告知对方发送的是文件信息，并让对方打开文件连接
    public static final String FILEINFORESPONSE = "FILEINFO R";  //PC告知本移动端已接受文件信息
    public static final String REPLACER = "${r}";  // \r占位符
    public static final String REPLACEN = "${n}";  // \n占位符

    public static CopyOnWriteArrayList<Uri> filesSendingQueue = new  CopyOnWriteArrayList<>(); // 文件发送队列
    public static String sendFileName; // 当前发送的文件名
    public static HashSet<String> filesSendedSet = new HashSet<>(); // 文件路径映射到是否已发送

    public static LinkedList<String> clipDatas = new LinkedList<>();

    public static void resetSendInfo(){
        filesSendingQueue.clear();
        synchronized (ConnectionInfo.class){
            filesSendedSet.clear();
        }
    }


}
