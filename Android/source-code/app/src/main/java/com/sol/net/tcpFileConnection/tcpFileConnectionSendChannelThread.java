package com.sol.net.tcpFileConnection;

import com.sol.net.ConnectionInfo;

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
            byte[] bytes = new byte[FILESENDBUFSIZE];
            int length ,sum=0,i=0;
            System.out.println("开始传输，文件大小："+file.length());
            while (-1!=(length = fis.read(bytes, 0, bytes.length))) { //每次发送至多FILESENDBUFSIZE字节的数据，实际发送数据取决于length
                out.write(bytes, 0, length);
                out.flush();
                sum+=length;
                progress = (1.0f*sum/(int)file.length())*100;
            }
            System.out.println("传输完成，发送字节数："+sum);
            ConnectionInfo.ifSended = true;  //发送完成，复位
        }catch (Exception e){
            e.printStackTrace();
            System.out.println("!!文件传输失败!!");
        }finally {
            try {
                fis.close(); //closeConnection();
            }catch (Exception e){}
            ConnectionInfo.filesSendFlag.put(filePath,true);
        }


    }



}
