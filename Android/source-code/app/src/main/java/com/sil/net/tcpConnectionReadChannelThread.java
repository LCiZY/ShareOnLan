package com.sil.net;

import com.sil.MainActivity;

import java.io.IOException;


public class tcpConnectionReadChannelThread extends tcpConnectionChannel {


    public String receiveStr = null;
    private Runnable uiRun;
    public tcpConnectionReadChannelThread(Runnable uiRun){
        this.uiRun = uiRun;
    }


    @Override
    public void run(){

        new Thread(uiRun).start(); //更新界面
        while (r_runFlag){

           try{ sleep(2000); } catch (InterruptedException e){ if(!r_runFlag) break; }
            if(!r_runFlag) break;

           String str ;
           //阻塞

           try{ str = in.readLine();  }catch (IOException e){
               tcpConnectionChannel.r_runFlag = false; establishFlag=false;  //e.printStackTrace();
               System.out.println("与服务器的连接超时");break;
           }
           if(str==null)  {
               tcpConnectionChannel.r_runFlag = false;
               System.out.println("与服务器的连接断开");
               establishFlag=false; break;
           }
            //将换行符复原
            str = str.replace(ConnectionInfo.REPLACEN,"\n");
            str = str.replace(ConnectionInfo.REPLACER,"\r");

            System.out.println("--------------------------------------------------接收数据："+str);
           if(!ConnectionInfo.RESPONSE.equals(str)&&ConnectionInfo.ifReceive&&str.trim().length()>0) {
               receiveStr = str;
               if(str.indexOf(ConnectionInfo.FILEINFOCONTROLMESSAGE)==0) {//PC端告知本移动端 要发送文件 并 发来文件信息
                   ConnectionInfo.processReceiveFileInfo(str);
                   MainActivity.instance.receiveFile();
               }
               if(str.contentEquals(ConnectionInfo.FILEINFORESPONSE))//PC端告知本移动端 已收到 文件名和文件大小的信息
                   MainActivity.instance.sendFile(ConnectionInfo.sendFileName);
               else
                   MainActivity.clipboard.setText(receiveStr);
           }

        }

        if(client != null)//如果构造函数建立起了连接，则关闭套接字，如果没有建立起连接，自然不用关闭
            try{client.close(); }catch (Exception e){establishFlag=false;}//只关闭socket，其关联的输入输出流也会被关闭
        establishFlag=false;
        System.out.println("---------------------------------与服务端断开连接-----------------------------------");

        new Thread(uiRun).start(); //更新界面


    }
}
