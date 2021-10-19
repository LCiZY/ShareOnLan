package com.sol.net;

import com.sol.MainActivity;

import java.io.IOException;

import static com.sol.MainActivity.enableClipChangeSend;
import static com.sol.MainActivity.disableClipChangeSend;
import static com.sol.MainActivity.handler;


public class tcpConnectionReadChannelThread extends tcpConnectionChannel {


    public String receiveStr = null;
    private Runnable uiRun;
    public tcpConnectionReadChannelThread(Runnable uiRun){
        this.uiRun = uiRun;
    }
    private int tick=0;

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

            System.out.print("tick:"+(tick++)+" ");
           if(!ConnectionInfo.RESPONSE.equals(str)) { //如果不是”心跳包“消息
               receiveStr = str;
               if(str.indexOf(ConnectionInfo.FILEINFOCONTROLMESSAGE)==0) {//PC端告知本移动端 要发送文件 并 发来文件信息
                   ConnectionInfo.processReceiveFileInfo(str);
                   MainActivity.instance.receiveFile();
               }else if(str.contentEquals(ConnectionInfo.FILEINFORESPONSE))//PC端告知本移动端 已收到 文件名和文件大小的信息
                   MainActivity.instance.sendFile(ConnectionInfo.sendFileName);
               else { //普通文本
                   System.out.println("----------------------接收文本："+str);
                   disableClipChangeSend();
                   MainActivity.clipboard.setText(receiveStr);
                   ConnectionInfo.clipDatas.addFirst(receiveStr);
                   handler.sendEmptyMessageDelayed(100,0);
                   enableClipChangeSend();
                   MainActivity.instance.toastOnUI("接收到来自PC的消息，已复制到剪贴板。");

               }
           }else if(ConnectionInfo.RESPONSE.equals(str)) {  //如果是”心跳包“消息，回应一个”心跳包“消息报文
               MainActivity.instance.sendMessage(ConnectionInfo.RESPONSE+"\n");
           }

        }

        if(client != null)//如果构造函数建立起了连接，则关闭套接字，如果没有建立起连接，自然不用关闭
            try{client.close(); }catch (Exception e){establishFlag=false;}//只关闭socket，其关联的输入输出流也会被关闭
        establishFlag=false;
        System.out.println("---------------------------------与服务端断开连接-----------------------------------");

        new Thread(uiRun).start(); //更新界面


    }
}
