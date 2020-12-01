package com.sol.net;


import java.util.HashMap;
import java.util.LinkedList;

public class ConnectionInfo {

    public static String PC_IP = null;
    public static boolean ifSended = true;  //已经发送打开的文件
    public static final String RESPONSE = "R";  //PC端接收到消息时返回R
    public static final String FILECONTROLMESSAGE = "${FILE}";   //PC告知本移动端要发送文件
    public static final String FILEINFOCONTROLMESSAGE = "${FILEINFO}";   //告知对方发送的是文件信息，并让对方打开文件连接
    public static final String FILEINFORESPONSE = "FILEINFO R";  //PC告知本移动端已接受文件信息
    public static final String REPLACER = "${r}";  //PC告知本移动端已接受文件信息
    public static final String REPLACEN = "${n}";  //PC告知本移动端已接受文件信息

    public static String sendFileName;
    public static HashMap<String,String> receiveFileInfo = new HashMap<>();
    public static HashMap<String,Boolean> filesSendFlag = new HashMap<>();
    public static LinkedList<String> clipDatas = new LinkedList<>();



    public static void processReceiveFileInfo(final String info){
        if(info==null) return;
        String[] infos = info.split("\\|");
        for (int i=1;i+1<infos.length;i+=2){
            receiveFileInfo.put(infos[i],infos[i+1]);
        }
    }

}
