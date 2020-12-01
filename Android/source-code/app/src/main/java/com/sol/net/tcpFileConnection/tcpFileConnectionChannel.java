package com.sol.net.tcpFileConnection;

import com.sol.net.tcpConnectionChannel;

import java.io.BufferedInputStream;
import java.io.BufferedReader;

import java.io.PrintStream;
import java.net.InetSocketAddress;
import java.net.Socket;


public class tcpFileConnectionChannel extends Thread {



    public static Socket fileClient = null;
    //获取Socket的输出流，用来发送数据到服务端
    public static PrintStream out = null;

    //获取Socket的输入流，用来接收从服务端发送过来的数据
    public static BufferedReader in = null;

    //获取socket字节流
    public static BufferedInputStream bis = null;

    public static boolean establishFlag = false;

    public static final int fileServerPort = 65534;

    public static final int FILERECEIVEBUFSIZE=512000;
    public static final int FILESENDBUFSIZE=102400;

    public static final int FILESOCKETOUTTIME = 10;

    public static float progress = 0;

    //只有另一条channel连上了才能连
    public static void connectTo() throws Exception{

        fileClient = new Socket();
        System.out.print("连接到ip："+tcpConnectionChannel.server_ip);
        System.out.print("  连接到端口："+fileServerPort);
        System.out.println("  设置超时时间："+FILESOCKETOUTTIME+"s");
        fileClient.connect(new InetSocketAddress(tcpConnectionChannel.server_ip,fileServerPort),tcpConnectionChannel.connectTimeOutValue*1000);
        if(fileClient==null)  { throw new Exception("error:socket error"); }

        //获取Socket的输出流，用来发送文件到服务端
        out = new PrintStream (fileClient.getOutputStream());
        //获取Socket的输入流，用来接收从服务端发送过来的数据
        bis = new BufferedInputStream(fileClient.getInputStream());

        //设置超时时间
       fileClient.setSoTimeout(FILESOCKETOUTTIME*1000);

        establishFlag = true;
    }



    public static void closeConnection(){
        if(fileClient!=null) { try{ fileClient.close();  }catch(Exception e){} }
        if(in!=null) { try{ in.close();  }catch(Exception e){} }
        if(out!=null) { try{ out.close();   }catch(Exception e){} }


        fileClient = null;
        out = null;
        in = null;
        establishFlag = false;
    }


    public static void closeSocket(){
        if(fileClient!=null) { try{ fileClient.close();  }catch(Exception e){} }
        establishFlag = false;
    }


}
