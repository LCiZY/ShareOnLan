package com.sil.net;



public class tcpConnectionWriteChannelThread extends tcpConnectionChannel {


    String msg = "";

    public void setMsg(String msg){
        this.msg = msg==null?"":msg;
    }


    @Override
    public void run(){

        while(w_runFlag){
            //阻塞当前线程，等点击按钮后再发送消息
            try {
                Thread.sleep(Integer.MAX_VALUE);
            } catch (InterruptedException e) {if(!w_runFlag) break; }

            if(!w_runFlag) break;

            //发送数据到服务端,如果当前剪贴板数据为空，则发送“R”，否则根据服务器代码来看，会断开连接
            if(ConnectionInfo.ifSend) {
                out.print(this.msg.length() == 0 ? ConnectionInfo.RESPONSE : this.msg);
                System.out.println("--------------------------------------------------发送数据：" + this.msg);
                this.setMsg("");
            }

        }

        if(client != null)//如果构造函数建立起了连接，则关闭套接字，如果没有建立起连接，自然不用关闭
            try{client.close(); }catch (Exception e){}


    }




}
