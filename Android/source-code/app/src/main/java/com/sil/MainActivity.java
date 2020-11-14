package com.sil;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.HorizontalScrollView;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.sil.adapter.FileListAdapter;
import com.sil.config.Config;
import com.sil.control.DetectThread;
import com.sil.net.ConnectionInfo;
import com.sil.net.UdpConnection.udpServer;
import com.sil.net.tcpConnectionChannel;
import com.sil.net.tcpConnectionReadChannelThread;
import com.sil.net.tcpConnectionWriteChannelThread;
import com.sil.net.tcpFileConnection.tcpFileConnectionChannel;
import com.sil.net.tcpFileConnection.tcpFileConnectionReceiveChannelThread;
import com.sil.net.tcpFileConnection.tcpFileConnectionSendChannelThread;
import com.sil.util.CheckArgumentUtil;
import com.sil.util.FileUtils;
import com.sil.util.JxdUtils;
import com.sil.util.getSystemInfo;
import com.sil.util.utils;

import java.io.File;
import java.io.IOException;
import java.net.SocketTimeoutException;
import java.util.ArrayList;


public class MainActivity extends AppCompatActivity {

    public static MainActivity instance= null;
    public static Context context = null;


    public static ClipboardManager clipboard ;
    public static ClipboardManager.OnPrimaryClipChangedListener clipChangedListener= null;

    public static int widthPixels;
    public static int heightPixels;




    private ImageButton autoConnectImgBtn;
    private  ImageView ifConnectImageView;
    private  TextView ifConnectTextView;
    private  EditText edit_ip;
    private  EditText edit_port;
    private HorizontalScrollView pagesScrollView;
    private LinearLayout pages;
    private LinearLayout page1;
    private LinearLayout page2;
    private touchOnPagesListener pagesListener;
    private  Button btn_send;
    private  CheckBox checkBox_ifEnableAutoSend;
    private  CheckBox checkBox_EnableSend;
    private  CheckBox checkBox_EnableReceive;
    private  TextView secretTextView;
    private  ListView fileListView;
    private  FileListAdapter fileListAdapter;
    private  ImageButton homeTab;
    private  ImageButton fileTab;


    //读、写线程
    private  tcpConnectionReadChannelThread readChannelThread;
    private  tcpConnectionWriteChannelThread writeChannelThread;

    public   static String serverIp = null;
    public  static int serverPort = 0;


    public DetectThread detectThread = null;
    public Runnable uiThread;  //改变UI使用的线程
    public Runnable uiSetIpPortRunnable;
    public static Runnable listViewRunnable;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        instance = this;
        context = getApplicationContext();

        System.out.println("------------------------------");
        System.out.println(this.getFilesDir().getAbsolutePath());

        System.out.println("------------------------------");

        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_main);

        //请求储存权限
        verifyStoragePermissions(MainActivity.this);
        //得到剪贴板
        clipboard = (ClipboardManager) this.getSystemService(Context.CLIPBOARD_SERVICE);
        //得到设备信息：如宽高
        getServiceInfo();

        pagesScrollView = findViewById(R.id.pagesScrollView);
        pages = findViewById(R.id.pages);
        page1 = findViewById(R.id.page1);
        page2 = findViewById(R.id.page2);
        autoConnectImgBtn = findViewById(R.id.autoConnectImgBtn);
        ifConnectImageView = findViewById(R.id.ifConnectImageView);
        ifConnectTextView = findViewById(R.id.ifConnectTextView);
        edit_ip = findViewById(R.id.edit_ip);
        edit_port = findViewById(R.id.edit_port);
        btn_send = findViewById(R.id.sendBtn);
        checkBox_EnableSend = findViewById(R.id.checkBox_EnableSend);
        checkBox_ifEnableAutoSend = findViewById(R.id.checkBox_ifEnableAutoSend);
        checkBox_EnableReceive = findViewById(R.id.checkBox_EnableReceive);
        secretTextView = findViewById(R.id.secret);
        fileListView=findViewById(R.id.fileList);
        homeTab = findViewById(R.id.homeTab);
        fileTab = findViewById(R.id.fileTab);

        if(checkBox_EnableReceive.isChecked()) on_checkBox_EnableReceiveChange(checkBox_EnableReceive);

        UIInit();


        setUpListener();

        runnableInit();

        detectThread = new DetectThread(uiThread);
        detectThread.start();

        new udpServer(uiSetIpPortRunnable).start();



    }

    @Override
    protected void onResume(){
        super.onResume();
        System.out.println("onResume----------------------");
        if(ConnectionInfo.ifSended) return;
        if(tcpConnectionChannel.establishFlag)
            checkOpenFile();
        else
            toastOnUI("网络异常：未连接至PC");
    }


    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        System.out.println("newIntent--------------------------");
        ConnectionInfo.ifSended = false;
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {

        if(ev.getAction() == MotionEvent.ACTION_DOWN){
            if(pagesListener!=null)
            setMotionEventX(pagesListener,ev.getX());
        }
        return super.dispatchTouchEvent(ev);
    }


    //点击发送按钮
    public void onSendBtnClicked(View view){
        if(!this.checkBox_EnableSend.isChecked()) return;

        if(!tcpConnectionChannel.establishFlag){
            toastOnUI("连接失败，检查IP、端口号或重试");
            return;
        }

        sendMessage(getSystemInfo.getClipboardContent(this));
        toastOnUI("发送成功~");

    }


    public void saveConfig(){
        //保存本次设置
        Config.setConfiguration("serverIp",this.edit_ip.getText().toString());
        Config.setConfiguration("serverPort",""+this.edit_port.getText().toString());
        Config.setConfiguration("enableAutoSend",""+this.checkBox_ifEnableAutoSend.isChecked());
        Config.writeConfigToFile();
    }

    private void sendMessage(String msg){
        if(writeChannelThread==null) return;
        writeChannelThread.setMsg(msg);
        writeChannelThread.interrupt();

    }



    private  boolean ifEditBoxIpPortValidate(){
        if(instance==null) return false;
        //从输入框得到IP和端口
        serverIp = instance.edit_ip.getText().toString(); try{ serverPort = Integer.parseInt(instance.edit_port.getText().toString());}catch (Exception e){serverPort=0;}
        //如果没填ip，则把可能对的ip填上去
        if(serverIp==null||"".contentEquals(serverIp)) {serverIp = ConnectionInfo.PC_IP;instance.edit_ip.setText(serverIp==null?"":serverIp);}
        //检查IP和端口号
        if(!CheckArgumentUtil.checkIfIpValidate(serverIp) ||serverPort<1025||serverPort>65533)
        {toastOnUI("请输入正确的IP地址和端口号。");return false;}
        return true;
    }

    //点击 imagebtn 的槽函数
    public void autoConnect(View view){
        if(DetectThread.detectFlag){
            DetectThread.detectFlag = false;
            DetectThread.interrupted();
            notifyDisconnect();
            toastOnUI("连接已关闭");
        }else{
            DetectThread.detectFlag = true;
            detectThread = new DetectThread(uiThread);
            detectThread.start();
            toastOnUI("正在尝试连接至目标主机。。。");
        }

    }

    //在detect线程中调用
    public  void ConnectionInit(){
        if(ifEditBoxIpPortValidate()) {
            tcpConnectionChannel.closeConnection();
            tryConnect(serverIp, serverPort);
        }
    }

    private  void tryConnect(String ip,int port){
        if(instance==null) return;
        try{
           tcpConnectionChannel.connectTo(ip,port);
           if(tcpConnectionChannel.establishFlag){
               if(instance.checkBox_EnableReceive.isChecked())
               { instance.readChannelThread = new tcpConnectionReadChannelThread(uiThread);instance.readChannelThread.start();  tcpConnectionChannel.r_runFlag = true; }
               if(instance.checkBox_EnableSend.isChecked())
               { instance.writeChannelThread = new tcpConnectionWriteChannelThread();instance.writeChannelThread.start();tcpConnectionChannel.w_runFlag = true;}
           }
        }
        catch (SocketTimeoutException e){
            tcpConnectionChannel.closeConnection();
          //  e.printStackTrace();
            System.out.println("---------------------------------尝试连接至目标主机失败：连接超时-----------------------------------");
        }catch (Exception e1){
            tcpConnectionChannel.closeConnection();
            e1.printStackTrace();
            System.out.println("---------------------------------尝试连接至目标主机失败：未知错误-----------------------------------");
        }
    }



    //点击关闭连接
    private  void notifyDisconnect(){
         try {
           if(tcpConnectionChannel.client!=null)
            tcpConnectionChannel.client.close();
        }catch (Exception e){}
        finally {
            tcpConnectionChannel.closeConnection();
        }
    }


    long preTime = 0;

    //是否监听剪贴板checkbox：自动发送
    public void enableAutoSend(View view){
        // 获取系统剪贴板
        saveConfig();
        if(clipboard==null) {toastOnUI("获取剪贴板失败"); return;}
        if(!checkBox_ifEnableAutoSend.isChecked()){  //取消了勾选，移除监听器并退出
            if(clipChangedListener != null)
            // 移除指定的剪贴板数据改变监听器
                System.out.println("移除监听器，退出");
            clipboard.removePrimaryClipChangedListener(clipChangedListener);
            return;
        }

        //勾选了，且不存在监听器方法实例
        if(clipChangedListener==null)
        clipChangedListener = new ClipboardManager.OnPrimaryClipChangedListener() {
            @Override
            public void onPrimaryClipChanged() {
                long now = System.currentTimeMillis();
                if(now-preTime<200) return;
                // 剪贴板中的数据被改变，此方法将被回调
                onSendBtnClicked(checkBox_ifEnableAutoSend);
                System.out.println("clipboard监听");
                preTime = now;
            }
        };
        // 添加剪贴板数据改变监听器
        clipboard.addPrimaryClipChangedListener(clipChangedListener);

    }

    //是否开启 发送数据到PC端的checkbox，本地客户端
    public void on_checkBox_EnableSendChange(View view){
        Config.setConfiguration("enableSend",checkBox_EnableSend.isChecked()+"");
        this.checkBox_ifEnableAutoSend.setChecked(!this.checkBox_EnableSend.isChecked());
        this.checkBox_ifEnableAutoSend.setEnabled(this.checkBox_EnableSend.isChecked());
        ConnectionInfo.ifSend = checkBox_EnableSend.isChecked();
    }

    //是否开启 从PC端接收数据的checkbox,本地服务端
    public void on_checkBox_EnableReceiveChange(View view){
        Config.setConfiguration("enableReceive",checkBox_EnableReceive.isChecked()+"");

        ConnectionInfo.ifReceive = checkBox_EnableReceive.isChecked();
    }



    /*
    把ui改变时的逻辑都写在此方法中
     */
    public void uiChange(){
        ifConnectTextView.setText(tcpConnectionChannel.establishFlag?"已连接":"未连接");
        ifConnectImageView.setImageLevel(tcpConnectionChannel.establishFlag?1:0);

        autoConnectImgBtn.setImageLevel(DetectThread.detectFlag?1:0);



    }

    public void uiSetIpPort(String ip,String port){
        serverIp = ip; serverIp = port;
        this.edit_ip.setText(ip);
        this.edit_port.setText(port);
    }



    public void toastOnUI(final String str){

        new Thread(new Runnable() {
            @Override
            public void run() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {

                        Toast.makeText(getApplicationContext(), str, Toast.LENGTH_SHORT).show();

                    }
                });
            }
        }).start();

    }



    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };
    public static void verifyStoragePermissions(Activity activity) {

        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,REQUEST_EXTERNAL_STORAGE);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void getServiceInfo(){
        DisplayMetrics outMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(outMetrics);
         widthPixels = outMetrics.widthPixels;
         heightPixels = outMetrics.heightPixels;
        System.out.println("--------------------width:" + widthPixels);
        System.out.println("--------------------height:" + heightPixels);
    }

    public void  checkOpenFile(){

        Intent intent = getIntent();
        String action = intent.getAction();
        String type = intent.getType();

        String[] file_paths ;
        if (TextUtils.equals(action, Intent.ACTION_VIEW)) {
            Uri uri = intent.getData();
            if(uri==null) return;
            file_paths = new String[1];
            System.out.println("单个文件打开");
            if (TextUtils.equals(uri.getScheme(), "file")) {
                file_paths[0] = JxdUtils.getPath(this.getApplicationContext(), uri);
                ConnectionInfo.filesSendFlag.put(file_paths[0],false);
            }else if (TextUtils.equals(uri.getScheme(), "content")) {
                file_paths[0] = JxdUtils.getPath(this.getApplicationContext(), uri);
                ConnectionInfo.filesSendFlag.put(file_paths[0],false);
            }
            notifySendFile(file_paths);
        }
        else if (Intent.ACTION_SEND.equals(action) && type != null) {
            file_paths = new String[1];

                Uri uri = (Uri) intent.getParcelableExtra(Intent.EXTRA_STREAM);
                System.out.println("单个文件发送");
                if (uri != null) {
                    file_paths[0] = JxdUtils.getPath(this.getApplicationContext(), uri);
                    ConnectionInfo.filesSendFlag.put(file_paths[0],false);
                    notifySendFile(file_paths);
                }

        } else if (Intent.ACTION_SEND_MULTIPLE.equals(action) && type != null) {

                ArrayList<Uri> uris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
                System.out.println("多个文件发送");
                if (uris != null) {
                    if (uris.size() > 1) {
                        file_paths = new String[uris.size()];
                        for(int i=0;i<uris.size();i++) {
                            file_paths[i] = JxdUtils.getPath(this.getApplicationContext(), uris.get(i));
                            ConnectionInfo.filesSendFlag.put(file_paths[i],false);
                        }
                        notifySendFile(file_paths);
                    }
                }

        }






    }

    public void notifySendFile(final String[] file_paths){
        new Thread(new Runnable() {
            @Override
            public void run() {
                for(int i=0;i<file_paths.length;i++) {
                    String file_path = file_paths[i];
                    System.out.println("文件路径：" + file_path);
                    ConnectionInfo.sendFileName = file_path;
                    File file;
                    try {
                        file = new File(file_path);if (!file.exists()) { toastOnUI("文件不存在");return; }
                    } catch (Exception e) { toastOnUI("文件不存在");return; }
                    if (instance == null) { toastOnUI("未知错误");return; }
                    //从输入框得到IP和端口
                    serverIp = instance.edit_ip.getText().toString();
                    //检查IP
                    if (!CheckArgumentUtil.checkIfIpValidate(serverIp)) { toastOnUI("请输入正确的IP地址。");return; }
                    //向PC端发送文件名和文件大小
                    //先发 "文件名"和"文件大小（字节）",如果有回应则发送文件
                    sendMessage(ConnectionInfo.FILEINFOCONTROLMESSAGE + utils.getFileInfo(file));
                    System.out.println("第"+(i+1)+"个文件开始发送");
                    while (ConnectionInfo.filesSendFlag.get(file_path)!=null&&!ConnectionInfo.filesSendFlag.get(file_path)){
                        try { Thread.sleep(100); } catch (InterruptedException e) { e.printStackTrace(); }
                    }
                    System.out.println("第"+(i+1)+"个文件发送完成");
                }
                ConnectionInfo.filesSendFlag.clear();
            }
        }).start();
    }


    public void receiveFile(){

        try {
            tcpFileConnectionChannel.connectTo();
            if(tcpFileConnectionChannel.establishFlag){
                new tcpFileConnectionReceiveChannelThread().start();
            }
        } catch (Exception e) {
            e.printStackTrace();
            tcpFileConnectionChannel.closeConnection();
        }


    }

    public void sendFile(final String filePath){

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    tcpFileConnectionChannel.connectTo();
                    if(tcpFileConnectionChannel.establishFlag){
                        new tcpFileConnectionSendChannelThread(filePath).start();
                    }
                }catch (IOException e){
                    toastOnUI("打开文件失败"); tcpFileConnectionChannel.closeConnection();
                }catch (Exception e){
                    e.printStackTrace();
                    toastOnUI( "无法连接至目标主机"); tcpFileConnectionChannel.closeConnection();
                }
            }
        }).start();


    }


    public void openFile(String filePath){
        System.out.println(filePath);
        System.out.println(Config.RECEIVEFILEDIRETORY);


       File file = new File(filePath);
       FileUtils.shareFile(this.getApplicationContext(),file);
    }



    public void UIInit(){
        //两个滑动页面初始化
        LinearLayout.LayoutParams layoutParams =(LinearLayout.LayoutParams) page1.getLayoutParams();
        layoutParams.width = widthPixels;// 控件的宽强制设成屏幕宽度
        layoutParams.height = new Double(heightPixels*190/470.0).intValue();
        page1.setLayoutParams(layoutParams); //使设置好的布局参数应用到控件

        LinearLayout.LayoutParams layoutParams2 =(LinearLayout.LayoutParams) page2.getLayoutParams();
        layoutParams2.width = widthPixels;// 控件的宽强制设成屏幕宽度
        layoutParams2.height = new Double(heightPixels*190/470.0).intValue();
        page2.setLayoutParams(layoutParams2); //使设置好的布局参数应用到控件

        //编辑框文字以及checkbox是否勾选
        edit_ip.setText(Config.getConfiguration("serverIp"));
        edit_port.setText(Config.getConfiguration("serverPort"));
        checkBox_EnableSend.setChecked(Config.getConfiguration("enableSend").contentEquals("true"));
        checkBox_ifEnableAutoSend.setChecked(Config.getConfiguration("enableAutoSend").contentEquals("true"));
        ConnectionInfo.ifSend=Config.getConfiguration("enableSend").contentEquals("true");
        this.checkBox_ifEnableAutoSend.setEnabled(this.checkBox_EnableSend.isChecked());
        if(this.checkBox_ifEnableAutoSend.isChecked()) enableAutoSend(this.checkBox_ifEnableAutoSend);
        checkBox_EnableReceive.setChecked(Config.getConfiguration("enableReceive").contentEquals("true"));
        ConnectionInfo.ifReceive = Config.getConfiguration("enableReceive").contentEquals("true");

        secretTextView.setText(Config.getConfiguration("secret"));


        //adapter
        fileListAdapter = new FileListAdapter(MainActivity.this,FileUtils.getFilesAllName(Config.RECEIVEFILEDIRETORY));
        fileListView.setAdapter(fileListAdapter);

        //底部菜单栏的按钮图标
        fileTab.setImageLevel(3);
    }


    private void setUpListener(){
        tabBtnListener tabbtnListener=new tabBtnListener();
        homeTab.setOnTouchListener(tabbtnListener);
        fileTab.setOnTouchListener(tabbtnListener);

        pagesListener = new touchOnPagesListener();
        pagesScrollView.setOnTouchListener(pagesListener);
        editTextEditListener editListener = new editTextEditListener();
        edit_ip.addTextChangedListener(editListener);
        edit_port.addTextChangedListener(editListener);

        fileListView.setOnItemClickListener(new fileListViewOnTouchListener());


    }


    public void runnableInit(){

        uiThread = new Runnable() {
            @Override
            public void run() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        uiChange();
                    }
                });
            }
        };
        uiSetIpPortRunnable = new Runnable() {
            @Override
            public void run() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        uiSetIpPort(udpServer.ip,udpServer.port);
                        System.out.println("已自动识别并设置IP与端口号");
                    }
                });
            }
        };

        listViewRunnable = new Runnable() {
            @Override
            public void run() {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        fileListAdapter.dataChange(FileUtils.getFilesAllName(Config.RECEIVEFILEDIRETORY));
                    }
                });
            }
        };
    }



    class tabBtnListener implements View.OnTouchListener {



        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if(v.getId() == homeTab.getId()){
                //按钮图标变换
                homeTab.setImageLevel(0);
                fileTab.setImageLevel(3);
                pagesScrollView.smoothScrollTo(0,0);

            }else if(v.getId() == fileTab.getId()){
                //按钮图标变换
                homeTab.setImageLevel(1);
                fileTab.setImageLevel(2);
                pagesScrollView.smoothScrollTo(widthPixels,0);

            }
            return true;
        }
    }

    public void setMotionEventX(touchOnPagesListener listener,float x){
        listener.startXPosition = x;
    }

    class touchOnPagesListener implements View.OnTouchListener {

        public float startXPosition;

        public float endXPosition;

        int currTab = 0;

        @Override
        public boolean onTouch(View v, MotionEvent event) {
//            System.out.println("touch-----------------------------------");
//            System.out.println(event.getX());
//            System.out.println(event.getY());
            switch (event.getAction()){
                case MotionEvent.ACTION_DOWN:
                    startXPosition = event.getX();
                    break;
                case  MotionEvent.ACTION_UP:
                    endXPosition = event.getX();
                    if(startXPosition-endXPosition>widthPixels/5) {
                        pagesScrollView.smoothScrollTo(widthPixels, 0);currTab = 1;
                        homeTab.setImageLevel(1);
                        fileTab.setImageLevel(2);
                        break;
                    }
                    if(endXPosition-startXPosition>widthPixels/5) {
                        pagesScrollView.smoothScrollTo(0, 0); currTab = 0;
                        homeTab.setImageLevel(0);
                        fileTab.setImageLevel(3);
                        break;
                    }
                    pagesScrollView.smoothScrollTo(currTab * widthPixels,0);

                    break;
                case  MotionEvent.ACTION_MOVE:
                    return false;
                default:
                    break;
            }
            return true;
        }
    }


    class editTextEditListener implements TextWatcher {


        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
        }

        @Override
        public void afterTextChanged(Editable s) {
            if(MainActivity.instance!=null){
               if(!"".contentEquals(MainActivity.instance.edit_port.getText().toString())&&!"".contentEquals(MainActivity.instance.edit_ip.getText().toString()))
               MainActivity.instance.saveConfig();
            }
        }
    }


    class fileListViewOnTouchListener implements AdapterView.OnItemClickListener{

            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                openFile((String)parent.getItemAtPosition(position));
            }

    }


    class doNothingListener implements View.OnTouchListener{

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            return true;
        }
    }









    private String getCurProcessName(Context context) {
        int pid = android.os.Process.myPid();
        ActivityManager activityManager = (ActivityManager)context.getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningAppProcessInfo appProcess : activityManager.getRunningAppProcesses()) {
            if (appProcess.pid == pid) {
                return appProcess.processName;
            }
        }
        return null;
    }
}



