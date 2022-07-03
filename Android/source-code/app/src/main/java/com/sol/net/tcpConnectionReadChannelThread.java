package com.sol.net;

import android.content.Context;
import android.util.Log;

import com.sol.MainActivity;
import com.sol.bean.FileInfo;
import com.sol.util.ToastUtils;

import java.io.IOException;

import static com.sol.MainActivity.disableClipChangeSend;
import static com.sol.MainActivity.enableClipChangeSend;


public class tcpConnectionReadChannelThread extends tcpConnectionChannel {

    private static final String TAG = "tcpConnectionReadChanne";

    public String receiveStr = null;
    private Context ctx;
    private Runnable uiRun;
    private MainActivity mainActivity;

    public tcpConnectionReadChannelThread(Context context, Runnable uiRun, MainActivity mainActivity) {
        ctx = context;
        this.uiRun = uiRun;
        this.mainActivity = mainActivity;
    }

    private int tick = 0;

    @Override
    public void run() {

        new Thread(uiRun).start(); //更新界面
        while (r_runFlag) {

            try {
                sleep(2000);
            } catch (InterruptedException e) {
                if (!r_runFlag) break;
            }
            if (!r_runFlag) break;

            String str;
            //阻塞

            try {
                str = in.readLine();
            } catch (IOException e) {
                tcpConnectionChannel.r_runFlag = false;
                establishFlag = false;  //e.printStackTrace();
                Log.d(TAG, "与服务器的连接超时");
                break;
            }
            if (str == null) {
                tcpConnectionChannel.r_runFlag = false;
                Log.d(TAG, "与服务器的连接断开");
                establishFlag = false;
                break;
            }
            //将换行符复原
            str = str.replace(ConnectionInfo.REPLACEN, "\n");
            str = str.replace(ConnectionInfo.REPLACER, "\r");

            System.out.print("tick:" + (tick++) + " ");
            if (!ConnectionInfo.RESPONSE.equals(str)) { //如果不是”心跳包“消息
                receiveStr = str;
                if (str.indexOf(ConnectionInfo.FILEINFOCONTROLMESSAGE) == 0) {//PC端告知本移动端 要发送文件 并 发来文件信息
                    mainActivity.receiveFile(FileInfo.processReceiveFileInfo(str));
                } else if (str.contentEquals(ConnectionInfo.FILEINFORESPONSE))//PC端告知本移动端 已收到 文件名和文件大小的信息
                    mainActivity.sendFile(ConnectionInfo.sendFileName);
                else { //普通文本
                    Log.d(TAG, "----------------------接收文本：" + str);
                    disableClipChangeSend();
                    MainActivity.clipboard.setText(receiveStr);
                    ConnectionInfo.clipDatas.addFirst(receiveStr);
                    mainActivity.getHandler().sendEmptyMessageDelayed(100, 0);
                    enableClipChangeSend();
                    ToastUtils.showToast(ctx, "接收到来自PC的消息，已复制到剪贴板。");

                }
            } else if (ConnectionInfo.RESPONSE.equals(str)) {  //如果是”心跳包“消息，回应一个”心跳包“消息报文
                mainActivity.sendMessage(ConnectionInfo.RESPONSE + "\n");
            }

        }

        if (client != null)//如果构造函数建立起了连接，则关闭套接字，如果没有建立起连接，自然不用关闭
            try {
                client.close();
            } catch (Exception e) {
                establishFlag = false;
            }//只关闭socket，其关联的输入输出流也会被关闭
        establishFlag = false;
        Log.d(TAG, "---------------------------------与服务端断开连接-----------------------------------");

        new Thread(uiRun).start(); //更新界面


    }
}
