package com.sol.net;


import static com.sol.MainActivity.handler;

public class tcpConnectionWriteChannelThread extends tcpConnectionChannel {


    String msg = "";

    public void setMsg(String msg){
        this.msg = msg==null?"":msg;
    }

    public void send(String msg){
        out.print(msg);
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

            out.print(this.msg.length() == 0 ? ConnectionInfo.RESPONSE : this.msg);
            if(!this.msg.contentEquals("R\n")) {
                //记录到剪贴板历史
                System.out.println("----------------发送文本：" + this.msg);
                try {
                    if (ConnectionInfo.clipDatas.isEmpty()||!this.msg.contentEquals(ConnectionInfo.clipDatas.getFirst())){
                        ConnectionInfo.clipDatas.addFirst(this.msg);
                        handler.sendEmptyMessageDelayed(100,0);
                    }
                } catch (Exception e) { }
            }

            this.setMsg("");


        }

        if(client != null)//如果构造函数建立起了连接，则关闭套接字，如果没有建立起连接，自然不用关闭
            try{client.close(); }catch (Exception e){}


    }




}
