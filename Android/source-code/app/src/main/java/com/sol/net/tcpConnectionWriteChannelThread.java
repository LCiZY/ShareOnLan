package com.sol.net;


import android.content.Context;
import android.os.Handler;
import android.util.Log;

import com.sol.MainActivity;
import com.sol.util.utils;

import java.util.LinkedList;

public class tcpConnectionWriteChannelThread extends tcpConnectionChannel {

    private static final String TAG = "tcpConnectionWriteChann";

    LinkedList<String> msgs = new LinkedList<>();
    private Context ctx;
    private Handler mainHandler;

    public tcpConnectionWriteChannelThread(Context context, Handler handler) {
        ctx = context;
        mainHandler = handler;
    }

    public void send(String msg) {
        synchronized (tcpConnectionWriteChannelThread.class) {
            msg = msg == null ? "" : msg;
            msgs.addLast(msg);
            this.interrupt();
        }
    }

    @Override
    public void run() {

        while (w_runFlag) {
            //阻塞当前线程，等点击按钮后再发送消息
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                if (!w_runFlag) break;
            }
            if (msgs.isEmpty()) continue;
            synchronized (tcpConnectionWriteChannelThread.class) {
                if (!w_runFlag) break;

                String msg = msgs.removeFirst();
                //发送数据到服务端,如果当前剪贴板数据为空，则发送“R”，否则根据服务器代码来看，会断开连接
                out.print(utils.isEmpty(msg) ? ConnectionInfo.RESPONSE : msg);
                out.flush();

                if (!msg.contentEquals(ConnectionInfo.RESPONSE+"\n")) {
                    Log.d(TAG, "----------------发送文本：" + msg);
                    //记录到剪贴板历史
                    if (msg.indexOf(ConnectionInfo.FILEINFOCONTROLMESSAGE) != 0) {
                        try {
                            if (ConnectionInfo.clipDatas.isEmpty() || !msg.contentEquals(ConnectionInfo.clipDatas.getFirst())) {
                                ConnectionInfo.clipDatas.addFirst(msg);
                                mainHandler.sendEmptyMessageDelayed(MainActivity.MSG_update_history_list, 0);
                            }
                        } catch (Exception ignored) { }
                    }

                }

            }

        }

        if (client != null)//如果构造函数建立起了连接，则关闭套接字，如果没有建立起连接，自然不用关闭
            try {
                client.close();
            } catch (Exception ignored) { }
    }


}
