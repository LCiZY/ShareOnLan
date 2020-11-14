package com.sil.net.tcpFileConnection;

import com.sil.MainActivity;
import com.sil.config.Config;
import com.sil.net.ConnectionInfo;
import com.sil.util.utils;

import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;

public class tcpFileConnectionReceiveChannelThread extends tcpFileConnectionChannel{

    private FileOutputStream fos;
    private DataInputStream dis ;

    @Override
    public void run(){
          try{

//              String fileInfo = in.readLine();
//              String[] fileInfos = fileInfo.split(" "); if(fileInfos.length<2) throw new Exception();
//              String fileName = fileInfos[0];
//              String fileSizeStr = fileInfos[1];
//              int fileSize = Integer.parseInt(fileSizeStr);
              String fileName = ConnectionInfo.receiveFileInfo.get("fileName");

                File directory = new File(Config.RECEIVEFILEDIRETORY);
                if(!directory.exists())
                    directory.mkdir();

              System.out.println("接收文件名："+fileName);
              System.out.println("储存路径："+Config.RECEIVEFILEDIRETORY);

                fos = new FileOutputStream(Config.RECEIVEFILEDIRETORY+fileName);
                dis = new DataInputStream(bis);


                System.out.println("======== 开始接收文件 ========");
                byte[] bytes = new byte[1];
                int length ;
                int sum = 0;
                while((length = dis.read(bytes)) != -1) {
                    sum+=length;
                    fos.write(bytes);
                }
                System.out.println("======== 接收字节数："+sum+" ====");
                System.out.println("======== 文件接收成功 ========");
        } catch (Exception e) {
                 e.printStackTrace();
        } finally {
              try {
                  if(fos != null)
                      fos.close();
                  if(dis != null)
                      dis.close();
                  closeConnection();
              } catch (IOException e) {
                  e.printStackTrace();
              }
              finally {
                  new Thread(MainActivity.listViewRunnable).start();
              }
        }



    }


}
