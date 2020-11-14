package com.sil.control;

import android.os.Looper;

import com.sil.MainActivity;
import com.sil.net.tcpConnectionChannel;

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
                System.out.println("尝试自动连接");
                if(ifFirstLoop){
                      Looper.prepare(); ifFirstLoop = false;
                }

                if(MainActivity.instance==null) {DetectThread.detectFlag = false;System.out.println("-----------------------------MainActivity.instance为null！！！");}
                MainActivity.instance.ConnectionInit();
                if(tcpConnectionChannel.establishFlag) new Thread(uiRun).start(); //更新界面

            }

            if (!detectFlag) break;
            try {
                sleep(5000);
            } catch (InterruptedException e) {
                if (!detectFlag) break;
            }
        }
    }




}
