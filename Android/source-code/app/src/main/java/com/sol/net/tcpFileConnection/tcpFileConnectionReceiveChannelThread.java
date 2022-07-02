package com.sol.net.tcpFileConnection;

import android.util.Log;

import com.sol.MainActivity;
import com.sol.bean.FileInfo;
import com.sol.config.Config;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Arrays;

import static com.sol.MainActivity.nextFileFlag;

public class tcpFileConnectionReceiveChannelThread extends tcpFileConnectionChannel {

    private static final String TAG = "tcpFileConnectionReceiv";

    private FileOutputStream fos;
    private DataInputStream dis;

    private final FileInfo info;

    public tcpFileConnectionReceiveChannelThread(FileInfo info) {
        this.info = info;
    }

    @Override
    public void run() {
        try {

            String fileName = info.fileName;

            File directory = new File(Config.RECEIVEFILEDIRETORY);
            if (!directory.exists()){
                boolean mkdir = directory.mkdir();
                if (!mkdir){
                    Log.e(TAG, "run: 创建目录失败!");
                    return;
                }
            }
            Log.d(TAG, "run: 接收文件名：" + fileName);
            Log.d(TAG, "run: 储存路径：" + Config.RECEIVEFILEDIRETORY);

            fos = new FileOutputStream(Config.RECEIVEFILEDIRETORY + fileName);
            dis = new DataInputStream(bis);

            Log.d(TAG, "run: ======== 开始接收文件:文件大小：" + info.fileSize + " ========");
            byte[] bytes = new byte[FILERECEIVEBUFSIZE];
            long length, sum = 0;
            while (!cancel) { //每次处理至多 FILERECEIVEBUFSIZE 字节的数据,实际处理数据量取决于length
                length = dis.read(bytes);
                if (length == -1) break;
                byte[] newBytes = Arrays.copyOf(bytes, (int) length);
                sum += length;
                fos.write(newBytes);
                progress = (1.0f * sum / info.fileSize) * 100;
            }
            Log.d(TAG, "run: ======== 文件接收成功,接收字节数：" + sum + "========");
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            try {
                if (fos != null)
                    fos.close();
                if (dis != null)
                    dis.close();
                closeConnection();
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                new Thread(MainActivity.listViewRunnable).start();
                nextFileFlag = true;
                transferDoneFlag = true;
                if (callback!=null) callback.run();
            }
        }


    }


}
