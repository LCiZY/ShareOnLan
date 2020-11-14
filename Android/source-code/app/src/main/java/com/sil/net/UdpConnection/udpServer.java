package com.sil.net.UdpConnection;

import com.sil.config.Config;
import com.sil.net.tcpConnectionChannel;
import com.sil.util.CheckArgumentUtil;
import com.sil.util.utils;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.SocketException;
import java.nio.charset.StandardCharsets;


public class udpServer extends Thread{

        public  static  boolean runFlag = true;
        public static final int udpListeningPort = 56789;

        public static String ip;
        public static String port;

        Runnable runnable;

        public udpServer(Runnable runnable){
            this.runnable = runnable;
        }

        @Override
        public void run() {
            //依然创建DatagramSocket类对象，因为此类表示用来发送和接收数据报包的套接字。
            //为了让捕获异常后的try代码块的finally可调用对象ds，故先创建对象，下面再建立实例
            DatagramSocket ds=null;

            try{
                ds = new DatagramSocket(56789);
                while (runFlag) {
                    System.out.println("开始监听udp数据包");
                    //创建数据报套接字并将其绑定到本地主机上的指定端口56789（对应发送端创建的端口）

                    //创建字节数组以做数据缓冲区
                    byte[] words = new byte[32];
                    //创建DatagramPacket类对象，并调用构造器用来接收长度为 length 的数据包
                    DatagramPacket dp = new DatagramPacket(words, 0, words.length);
                    //调用DatagramSocket类方法receive()接收数据报包
                    ds.receive(dp);
                    //再将数据报包转换成字节数组
                    byte[] data = dp.getData();
                    //通过使用平台的默认字符集解码data字节数组,方便打印输入
                    String str = new String(data, StandardCharsets.US_ASCII);

                    if(str==null) continue;
                    for(int i=str.length()-1;i>=0;i--){
                       if(0!=(int)str.charAt(i)){str=str.substring(0,i+1); break;}
                    }
                    System.out.println("udp数据包--"+str+"----");
                    str = utils.decrypt(str);
                    System.out.print("解密数据：");
                    System.out.println("----"+str+"----");
                    String[] strs = str.split(" ");
                    if(strs.length!=3) continue;
                    //对str解码得到ip地址和端口号
                    if(!CheckArgumentUtil.checkIfIpValidate(strs[0])) continue;
                    ip = strs[0];
                    port= strs[1];
                    if(!Config.getConfiguration("secret").contentEquals(strs[2])) continue;
                    //如果没连接上，跑ui线程
                    if(!tcpConnectionChannel.establishFlag){
                        new Thread(runnable).start();
                    }

                }
            } catch (SocketException e) {
                e.printStackTrace();
            }catch (IOException e) {
                e.printStackTrace();
            }finally{
                if(ds!=null){
                    ds.close();
                }
                System.out.println("-----------------------------------------------udp监听线程退出");
            }


    }
}
