package com.sol.control;

import android.os.Looper;

import com.sol.MainActivity;
import com.sol.net.tcpConnectionChannel;

public class DetectThread extends Thread{

    private boolean ifFirstLoop = true;

    public static boolean detectFlag = true;
    private Runnable uiRun;
    public DetectThread(Runnable uiRun){
        this.uiRun = uiRun;
    }

    @Override
    public void run(){
        ifFirstLoop = true;
        while (detectFlag) {
         //   System.out.println("检测连接状态:"+tcpConnectionChannel.establishFlag);
            if(!tcpConnectionChannel.establishFlag) {
                System.out.println("尝试自动连接至 "+MainActivity.serverIp+":"+MainActivity.serverPort);
                if(ifFirstLoop){
                      Looper.prepare(); ifFirstLoop = false;
                }

                if(MainActivity.instance==null) {DetectThread.detectFlag = false;System.out.println("-----------------------------MainActivity.instance为null！！！");}
                MainActivity.instance.ConnectionInit();
                if(tcpConnectionChannel.establishFlag) new Thread(uiRun).start(); //连接成功，更新界面

            }

            if (!detectFlag) break;
            try {
                sleep(5000);
            } catch (InterruptedException e) {
                if (!detectFlag)  break;
            }
        }
    }




}
