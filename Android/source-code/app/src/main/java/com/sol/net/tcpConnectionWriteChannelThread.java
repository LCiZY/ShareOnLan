package com.sol.net;


import android.content.Context;
import android.util.Log;

import com.sol.util.utils;

import java.util.LinkedList;

import static com.sol.MainActivity.handler;

public class tcpConnectionWriteChannelThread extends tcpConnectionChannel {

    private static final String TAG = "tcpConnectionWriteChann";

    LinkedList<String> msgs = new LinkedList<>();
    private Context ctx;

    public tcpConnectionWriteChannelThread(Context context) {
        ctx = context;
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

                if (!msg.contentEquals("R\n")) {
                    //记录到剪贴板历史
                    Log.d(TAG, "----------------发送文本：" + msg);
                    try {
                        if (ConnectionInfo.clipDatas.isEmpty() || !msg.contentEquals(ConnectionInfo.clipDatas.getFirst())) {
                            ConnectionInfo.clipDatas.addFirst(msg);
                            handler.sendEmptyMessageDelayed(100, 0);
                        }
                    } catch (Exception e) {
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
