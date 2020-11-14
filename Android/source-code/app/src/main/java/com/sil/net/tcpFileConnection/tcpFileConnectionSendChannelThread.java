package com.sil.net.tcpFileConnection;

import com.sil.net.ConnectionInfo;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

public class tcpFileConnectionSendChannelThread extends tcpFileConnectionChannel {

    private String filePath;
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
            System.out.println("======== 开始传输文件 ========");
            byte[] bytes = new byte[1024];
            int length ,sum=0,i=0;
            System.out.println("文件大小："+file.length());
            while (-1!=(length = fis.read(bytes, 0, bytes.length))) {
                out.write(bytes, 0, length);
                out.flush();
                sum+=length;
            }
            System.out.println("发送字节数："+sum);
            System.out.println("======== 文件传输成功 ========");
            ConnectionInfo.ifSended = true;
        }catch (Exception e){
            e.printStackTrace();
            System.out.println("======== 文件传输失败 ========");
        }finally {
            try {
                fis.close(); //closeConnection();
            }catch (Exception e){}
            ConnectionInfo.filesSendFlag.put(filePath,true);
        }


    }



}
