package com.sol.net;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.InetSocketAddress;
import java.net.Socket;

public class tcpConnectionChannel extends Thread{



    public static final int connectTimeOutValue = 3;  //数据读取超时时间
    public static final int readTimeOutValue = 10;  //数据读取超时时间
    public static boolean r_runFlag = false;
    public static boolean w_runFlag = false;
    public static boolean establishFlag = false;
    public static String server_ip = null;
    public static int server_port = 0;
    public static Socket client = null;



    //获取Socket的输出流，用来发送数据到服务端
    public static PrintStream out = null;

    //获取Socket的输入流，用来接收从服务端发送过来的数据
    public static BufferedReader in = null;

    public static void connectTo(String server_ip,int server_port) throws Exception{

        client = new Socket();
        client.connect(new InetSocketAddress(server_ip,server_port),connectTimeOutValue*1000);
        if(client==null)  { throw new Exception("error:socket error"); }
        tcpConnectionChannel.server_ip = server_ip;
        tcpConnectionChannel.server_port = server_port;

        //获取Socket的输出流，用来发送数据到服务端
        out = new PrintStream(client.getOutputStream());
        //获取Socket的输入流，用来接收从服务端发送过来的数据
        in =  new BufferedReader(new InputStreamReader(client.getInputStream()));

        //设置超时时间
        client.setSoTimeout(readTimeOutValue*1000);

        establishFlag = true;
    }

    public static void closeConnection(){
        tcpConnectionChannel.r_runFlag = false;
        tcpConnectionChannel.w_runFlag = false;
        tcpConnectionChannel.establishFlag=false;
        tcpConnectionChannel.server_ip = null;
        tcpConnectionChannel.server_port = 0;
        tcpConnectionChannel.client = null;
        tcpConnectionChannel.out=null;
        tcpConnectionChannel.in=null;
    }

}
