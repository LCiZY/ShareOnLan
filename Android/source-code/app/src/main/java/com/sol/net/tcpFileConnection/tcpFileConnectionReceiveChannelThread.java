package com.sol.net.tcpFileConnection;

import com.sol.MainActivity;
import com.sol.config.Config;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Arrays;

import static com.sol.MainActivity.nextFileFlag;
import static com.sol.net.ConnectionInfo.receiveFileInfo;

public class tcpFileConnectionReceiveChannelThread extends tcpFileConnectionChannel{

    private FileOutputStream fos;
    private DataInputStream dis ;

    @Override
    public void run(){
          try{

              String fileName = receiveFileInfo.get("fileName");

                File directory = new File(Config.RECEIVEFILEDIRETORY);
                if(!directory.exists())
                    directory.mkdir();

              System.out.println("接收文件名："+fileName);
              System.out.println("储存路径："+Config.RECEIVEFILEDIRETORY);

                fos = new FileOutputStream(Config.RECEIVEFILEDIRETORY+fileName);
                dis = new DataInputStream(bis);


                System.out.println("======== 开始接收文件:文件大小："+receiveFileInfo.get("fileSize")+" ========");
                byte[] bytes = new byte[FILERECEIVEBUFSIZE];
                int length ;
                int sum = 0;
                while(!cancel) { //每次处理至多 FILERECEIVEBUFSIZE 字节的数据,实际处理数据量取决于length
                    length = dis.read(bytes); if(length==-1) break;
                    byte[] newBytes = Arrays.copyOf(bytes,length);
                    sum+=length;
                    fos.write(newBytes);
                    progress = (1.0f*sum/Integer.parseInt(receiveFileInfo.get("fileSize")))*100;
                }
                System.out.println("======== 文件接收成功,接收字节数："+sum+ "========");
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
                  nextFileFlag = true;
              }
        }



    }


}
