package com.sol.net.tcpFileConnection;

import android.util.Log;

import com.sol.net.ConnectionInfo;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

public class tcpFileConnectionSendChannelThread extends tcpFileConnectionChannel {

    private static final String TAG = "tcpFileConnectionSendCh";

    private final String filePath;
    private FileInputStream fis;

    public tcpFileConnectionSendChannelThread(String filePath) {
        this.filePath = filePath;
    }

    @Override
    public void run(){

        try {
            File file = new File(filePath);
            if (!file.exists()) throw new IOException();

            fis = new FileInputStream(file);

            // 开始传输文件
            byte[] bytes = new byte[FILESENDBUFSIZE];
            long length ,sum=0,i=0;
            Log.d(TAG, "开始传输，文件大小："+file.length());
            while (-1!=(length = fis.read(bytes, 0, bytes.length)) && !cancel) { //每次发送至多FILESENDBUFSIZE字节的数据，实际发送数据取决于length
                out.write(bytes, 0, (int) length);
                out.flush();
                sum+=length;
                progress = (1.0f*sum/(int)file.length())*100;
            }
            Log.d(TAG, "传输完成，发送字节数："+sum);
        }catch (Exception e){
            e.printStackTrace();
            Log.d(TAG, "!!文件传输失败!!");
        }finally {
            try {
                Thread.sleep(1000);
                fis.close(); //closeConnection();
            }catch (Exception ignored){}
            finally {
                transferDoneFlag = true;
            }
            synchronized (ConnectionInfo.class) {
                ConnectionInfo.filesSendedSet.add(filePath);
            }
        }


    }



}
