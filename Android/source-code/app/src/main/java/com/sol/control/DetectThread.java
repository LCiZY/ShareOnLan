package com.sol.control;

import android.os.Looper;
import android.util.Log;

import com.sol.MainActivity;
import com.sol.net.tcpConnectionChannel;

public class DetectThread extends Thread{

    private static final String TAG = "DetectThread";

    private boolean ifFirstLoop = true;

    public static boolean detectFlag = true;

    private MainActivity mainActivity;
    public DetectThread(MainActivity mainActivity){
        this.mainActivity = mainActivity;
    }

    @Override
    public void run(){
        ifFirstLoop = true;
        while (detectFlag) {
         //   Log.d(TAG, "检测连接状态:"+tcpConnectionChannel.establishFlag);
            if(!tcpConnectionChannel.establishFlag) {
                Log.d(TAG, "尝试自动连接至 "+MainActivity.serverIp+":"+MainActivity.serverPort);
                if(ifFirstLoop){
                      Looper.prepare(); ifFirstLoop = false;
                }

                if(mainActivity==null) {
                    DetectThread.detectFlag = false;
                    continue;
                }
                mainActivity.ConnectionInit(null, null, null);

            }

            if (!detectFlag) break;
            try {
                sleep(500);
            } catch (InterruptedException e) {
                if (!detectFlag)  break;
            }
        }
    }




}
