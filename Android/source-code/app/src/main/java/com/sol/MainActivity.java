package com.sol;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.drawable.ColorDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
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

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.sol.adapter.FileListAdapter;
import com.sol.component.FlikerProgressBar;
import com.sol.component.PopupMenu;
import com.sol.component.bottomFileOpDialog;
import com.sol.component.circlerefresh.CircleRefreshLayout;
import com.sol.component.spinkit.SpinKitView;
import com.sol.config.Config;
import com.sol.control.DetectThread;
import com.sol.net.ConnectionInfo;
import com.sol.net.UdpConnection.udpServer;
import com.sol.net.tcpConnectionChannel;
import com.sol.net.tcpConnectionReadChannelThread;
import com.sol.net.tcpConnectionWriteChannelThread;
import com.sol.net.tcpFileConnection.tcpFileConnectionChannel;
import com.sol.net.tcpFileConnection.tcpFileConnectionReceiveChannelThread;
import com.sol.net.tcpFileConnection.tcpFileConnectionSendChannelThread;
import com.sol.util.CheckArgumentUtil;
import com.sol.util.FileUtils;
import com.sol.util.ToastUtils;
import com.sol.util.UIUtils;
import com.sol.util.getSystemInfo;
import com.sol.util.utils;

import java.io.File;
import java.io.IOException;
import java.net.SocketTimeoutException;
import java.util.ArrayList;
import java.util.concurrent.CopyOnWriteArrayList;

import static com.sol.net.ConnectionInfo.clipDatas;
import static com.sol.net.ConnectionInfo.receiveFileInfo;


public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";

    public static ClipboardManager clipboard;
    public static boolean ifClipChangedSend = true, if_shield_clip_detect = false;

    public static int widthPixels;
    public static int heightPixels;


    private PopupMenu popupMenu;
    private ImageView settingIcon;
    private ImageButton autoConnectImgBtn;
    private SpinKitView connnecting_ani;
    private ImageView ifConnectImageView;
    private TextView ifConnectTextView;
    private EditText edit_ip;
    private EditText edit_port;
    private HorizontalScrollView pagesScrollView;
    private LinearLayout pages;
    private LinearLayout page1;
    private LinearLayout page2;
    private touchOnPagesListener pagesListener;
    private Button btn_send;
    private CheckBox checkBox_ifEnableAutoSend;
    private TextView secretTextView;
    private CircleRefreshLayout mRefreshLayout;
    private TextView noFileTextView;
    private ListView fileListView;
    private FileListAdapter fileListAdapter;
    private ImageButton homeTab;
    private ImageButton fileTab;


    public tcpFileConnectionChannel sendFileConnection;
    public tcpFileConnectionChannel receiveFileConnection;
    //读、写线程
    private tcpConnectionReadChannelThread readChannelThread;
    private tcpConnectionWriteChannelThread writeChannelThread;

    public static String serverIp = null;
    public static int serverPort = 0;


    public DetectThread detectThread = null;
    public Runnable uiThread;  //改变UI使用的线程
    public Runnable uiSetIpPortRunnable;
    public static Runnable listViewRunnable;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!isTaskRoot()) {
            // Android launched another instance of the root activity into an existing task
            //  so just quietly finish and go away, dropping the user back into the activity
            //  at the top of the stack (ie: the last state of this task)
            Intent intent = getIntent();
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            finish();
            this.startActivity(intent);
            return;
        }

        Config.conFigInit(this);

        System.out.println("------------------------------");
        System.out.println(this.getFilesDir().getAbsolutePath());
        System.out.println("------------------------------");

        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_main);

        //请求储存权限
//        verifyStoragePermissions(MainActivity.this);
        requestReadWriteFilePermissions();

        //得到剪贴板
        clipboard = (ClipboardManager) this.getSystemService(Context.CLIPBOARD_SERVICE);
        //得到设备信息：如宽高
        getServiceInfo();

        popupMenu = new PopupMenu(this);
        settingIcon = findViewById(R.id.more_dots);
        pagesScrollView = findViewById(R.id.pagesScrollView);
        pages = findViewById(R.id.pages);
        page1 = findViewById(R.id.page1);
        page2 = findViewById(R.id.page2);
        autoConnectImgBtn = findViewById(R.id.autoConnectImgBtn);
        connnecting_ani = findViewById(R.id.connnecting_ani);
        ifConnectImageView = findViewById(R.id.ifConnectImageView);
        ifConnectTextView = findViewById(R.id.ifConnectTextView);
        edit_ip = findViewById(R.id.edit_ip);
        edit_port = findViewById(R.id.edit_port);
        btn_send = findViewById(R.id.sendBtn);
        checkBox_ifEnableAutoSend = findViewById(R.id.checkBox_ifEnableAutoSend);

        secretTextView = findViewById(R.id.secret);
        mRefreshLayout = findViewById(R.id.refresh_layout);
        fileListView = findViewById(R.id.fileList);
        noFileTextView = findViewById(R.id.noFileTextView);
        homeTab = findViewById(R.id.homeTab);
        fileTab = findViewById(R.id.fileTab);

        UIInit();


        setUpListener();

        runnableInit();

        popup_menu_window_init();

        new udpServer(uiSetIpPortRunnable).start();


        uiChange(); //初始化连接状态部分的界面状态
        System.out.println("created-------------------------");
        System.out.println("android.os.Process.myPid(): " + android.os.Process.myPid());
    }

    @Override
    protected void onResume() {
        super.onResume();
        System.out.println("onResume----------------------");

        new Thread(new Runnable() {
            @Override
            public void run() {
                ConnectionInit(new Runnable() {
                    @Override
                    public void run() {
                        checkOpenFile();
                        if (checkBox_ifEnableAutoSend.isChecked()) checkClipBoardText(); //检测剪贴板的文字
                    }
                }, new Runnable() {
                    @Override
                    public void run() {
                        toastOnUI("网络异常：未连接至PC");
                    }
                }, new DetectThread(MainActivity.this));
            }
        }).start();

    }


    @Override
    protected void onNewIntent(Intent intent) { //早于onResume
        super.onNewIntent(intent);
        setIntent(intent);
        System.out.println("newIntent--------------------------");
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        //如果是返回键
        if(keyCode== KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0){
            //重写返回键，让其跟按下home键具有相同效果
            Intent intent= new Intent(Intent.ACTION_MAIN);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.addCategory(Intent.CATEGORY_HOME);
            startActivity(intent);
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    private float startX,startY;

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {

        if (ev.getAction() == MotionEvent.ACTION_DOWN) {
            startX = ev.getX();
            startY = ev.getY();
            if (pagesListener != null)
                setMotionEventX(pagesListener, ev.getX());
            if (edit_ip != null && edit_port != null){
                if (edit_ip.hasFocus()) closeInputMethod(edit_ip);
                if (edit_port.hasFocus()) closeInputMethod(edit_port);
            }
        }
        return super.dispatchTouchEvent(ev);
    }

    private boolean closeInputMethod(EditText editText) {
        editText.clearFocus();
        InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
        boolean isOpen = imm.isActive();
        if (isOpen) {
            // imm.toggleSoftInput(0, InputMethodManager.HIDE_NOT_ALWAYS);//没有显示则显示
            imm.hideSoftInputFromWindow(editText.getWindowToken(), InputMethodManager.HIDE_NOT_ALWAYS);
        }
        return isOpen;
    }

    //点击发送按钮
    public void onSendBtnClicked(View view) {
        final Context self = this;
        new Thread(new Runnable() {
            @Override
            public void run() {
                ConnectionInit(new Runnable() {
                    @Override
                    public void run() {
                        sendMessage(getSystemInfo.getClipboardContent(self));
                        toastOnUI("剪贴板文本发送成功~");
                    }
                }, new Runnable() {
                    @Override
                    public void run() {
                        toastOnUI("连接失败，检查IP、端口号或重试");
                    }
                }, null);
            }
        }).start();

    }

    public void onSendFileBtnClicked(View view) {
        final AlertDialog alertDialog = new AlertDialog.Builder(this)
                .setTitle("发送文件至电脑")
                .setIcon(R.drawable.fly)
                .setView(R.layout.send_file_specification)
                .create();
        alertDialog.getWindow().setBackgroundDrawable(new ColorDrawable(getResources().getColor(R.color.bgColor)));
        alertDialog.setButton(DialogInterface.BUTTON_POSITIVE,"确定", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        alertDialog.dismiss();
                    }
                });
        alertDialog.show();
        UIUtils.setDialogTitleColor(alertDialog, getResources().getColor(R.color.textColor));

    }


    public void saveConfig() {
        //保存本次设置
        Config.setConfiguration("serverIp", this.edit_ip.getText().toString());
        Config.setConfiguration("serverPort", "" + this.edit_port.getText().toString());
        Config.setConfiguration("enableAutoSend", "" + this.checkBox_ifEnableAutoSend.isChecked());
        Config.writeConfigToFile();
    }

    public void sendMessage(String msg) {
        if (writeChannelThread == null) return;
        writeChannelThread.setMsg(msg);
        writeChannelThread.interrupt();

    }

    static int incorrectTimes = 0;
    private boolean ifEditBoxIpPortValidate() {
        //检查IP和端口号
        if (!CheckArgumentUtil.checkIfIpValidate(serverIp) || serverPort < 1025 || serverPort > 65533) {
            if (incorrectTimes++ % 20 == 0)
                toastOnUI("请输入正确的IP地址和端口号。");
            return false;
        }
        return true;
    }

    //点击 imagebtn 的槽函数
    public void autoConnect(View view) {
        if (DetectThread.detectFlag) {
            DetectThread.detectFlag = false;
            DetectThread.interrupted();
            closeConnection();
            toastOnUI("连接已关闭");
        } else {
            DetectThread.detectFlag = true;
            detectThread = new DetectThread(this);
            detectThread.start();
            toastOnUI("尝试连接至目标主机...");
        }
        uiChange();
    }

    //需要尝试连接时调用，比如在detect线程中调用, 连接成功调用successCallback方法,连接失败调用failCallback方法，不管成功失败都调用callback
    public void ConnectionInit(Runnable successCallback, Runnable failCallback, Runnable callback) {
        if (tcpConnectionChannel.establishFlag) {
            new Thread(uiThread).start();
            if (successCallback != null)
                new Thread(successCallback).start(); //建立连接后的回调
            return;
        }
        if (ifEditBoxIpPortValidate()) {
            tcpConnectionChannel.closeConnection();
            tryConnect(serverIp, serverPort);
        }
        if (tcpConnectionChannel.establishFlag) {
            new Thread(uiThread).start();
            if (successCallback != null)
                new Thread(successCallback).start(); //建立连接后的回调
        }else{
            if (failCallback != null)
                new Thread(failCallback).start(); //建立连接失败后的回调
        }
        if (callback != null)
            new Thread(callback).start();
    }

    private void tryConnect(String ip, int port) {
        try {
            tcpConnectionChannel.connectTo(ip, port);
            if (tcpConnectionChannel.establishFlag) {
                readChannelThread = new tcpConnectionReadChannelThread(this, uiThread, MainActivity.this);
                readChannelThread.start();
                tcpConnectionChannel.r_runFlag = true;
                writeChannelThread = new tcpConnectionWriteChannelThread(this);
                writeChannelThread.start();
                tcpConnectionChannel.w_runFlag = true;
            }
        } catch (SocketTimeoutException e) {
            tcpConnectionChannel.closeConnection();
            //  e.printStackTrace();
            System.out.println("---------------------------------尝试连接至目标主机失败：连接超时-----------------------------------");
            System.out.println(e.getMessage());
        } catch (Exception e1) {
            tcpConnectionChannel.closeConnection();
            e1.printStackTrace();
            System.out.println("---------------------------------尝试连接至目标主机失败：未知错误-----------------------------------");
        }
    }


    //点击关闭连接
    private void closeConnection() {
        try {
            if (tcpConnectionChannel.client != null)
                tcpConnectionChannel.client.close();
        } catch (Exception e) {
        } finally {
            tcpConnectionChannel.closeConnection();
        }
    }


    public static void enableClipChangeSend() {
        if_shield_clip_detect = false;
    }

    public static void disableClipChangeSend() {
        if_shield_clip_detect = true;
    }


    //是否监听剪贴板checkbox：自动发送
    public void enableAutoSend(View view) {
        // 获取系统剪贴板
        saveConfig();
        ifClipChangedSend = checkBox_ifEnableAutoSend.isChecked();
    }


    /*
    把ui改变时的逻辑都写在此方法中
     */
    public void uiChange() {
        ifConnectImageView.setImageLevel(tcpConnectionChannel.establishFlag ? 1 : 0);
        if (DetectThread.detectFlag) {
            autoConnectImgBtn.setImageLevel(1);
            if (tcpConnectionChannel.establishFlag) {
                ifConnectTextView.setText("已连接");
                ifConnectImageView.setVisibility(View.VISIBLE);
                connnecting_ani.setVisibility(View.GONE);
            } else {
                ifConnectTextView.setText("正在尝试连接");
                ifConnectImageView.setVisibility(View.GONE);
                connnecting_ani.setVisibility(View.VISIBLE);
            }

        } else {
            autoConnectImgBtn.setImageLevel(0);
            ifConnectTextView.setText("未连接");
            ifConnectImageView.setVisibility(View.VISIBLE);
            connnecting_ani.setVisibility(View.GONE);
        }

    }

    public void uiSetIpPort(String ip, String port) {
        serverIp = ip;
        serverIp = port;
        this.edit_ip.setText(ip);
        this.edit_port.setText(port);
    }

    public void toastOnUI(final String str) {
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

    public static final int REQUEST_CODE_ON_CREATE_REQUEST_PERMISSIONS = 100;

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        boolean granted = true;
        int i = 0;
        for (int grantResult : grantResults) {
            if (grantResult != PackageManager.PERMISSION_GRANTED) {
                granted = false;
                System.out.println("permission:" + permissions[i] + "   没有授权:" + grantResult);
                break;
            }
            i++;
        }
        if (granted) {
            ToastUtils.showToast(this, getString(R.string.text_permissions_granted));
            if (requestCode == REQUEST_CODE_ON_CREATE_REQUEST_PERMISSIONS){

            }
        } else {
            if (requestCode == REQUEST_CODE_ON_CREATE_REQUEST_PERMISSIONS) {
                ToastUtils.showToast(this, getString(R.string.text_no_permissions));
                finish();
            }
        }
    }

    private boolean requestReadWriteFilePermissions() {
        String[] permissions = new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE};
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            permissions = new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE};
        }
        return myRequestPermissions(this, permissions, REQUEST_CODE_ON_CREATE_REQUEST_PERMISSIONS);
    }

    public static boolean myRequestPermissions(Activity activity, String[] permissions, int requestCode) {
        if (permissions == null || permissions.length == 0) return true;
        boolean granted = true;
        for (String s : permissions) {
            if (ContextCompat.checkSelfPermission(activity, s) != PackageManager.PERMISSION_GRANTED) {
                granted = false;
            }
        }
        if (!granted) //没权限则发起请求
            ActivityCompat.requestPermissions(activity, permissions, requestCode);
        return granted;
    }


    public void getServiceInfo() {
        DisplayMetrics outMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(outMetrics);
        widthPixels = outMetrics.widthPixels;
        heightPixels = outMetrics.heightPixels;
        System.out.println("--------------------width:" + widthPixels);
        System.out.println("--------------------height:" + heightPixels);
    }

    /**
     * 检测intent中携带的文件，如果不在发送队列中则发送
     * */
    public void checkOpenFile() {

        Intent intent = getIntent();
        if (intent == null) return;

        String action = intent.getAction();
        String type = intent.getType();
        int currQueueSize =  ConnectionInfo.filesSendingQueue.size();

        System.out.println("--------------action:" + action);


        if (TextUtils.equals(action, Intent.ACTION_VIEW)) {
            Uri uri = intent.getData();
            if (uri == null) return;
            System.out.println("单个文件打开");
            if (TextUtils.equals(uri.getScheme(), "file") || TextUtils.equals(uri.getScheme(), "content")) {
                if (ConnectionInfo.filesSendingQueue.contains(uri)) return;
                ConnectionInfo.filesSendingQueue.add(uri);

            }
        } else if (Intent.ACTION_SEND.equals(action) && type != null) {

            Uri uri = (Uri) intent.getParcelableExtra(Intent.EXTRA_STREAM);
            System.out.println("单个文件发送");
            if (uri == null) return;
            if (ConnectionInfo.filesSendingQueue.contains(uri)) return;
            ConnectionInfo.filesSendingQueue.add(uri);
        } else if (Intent.ACTION_SEND_MULTIPLE.equals(action) && type != null) {

            ArrayList<Uri> uris = intent.getParcelableArrayListExtra(Intent.EXTRA_STREAM);
            System.out.println("多个文件发送");
            if (uris == null || uris.size() == 0) return;
            for (int i = 0; i < uris.size(); i++) {
                Uri uri = uris.get(i);
                if (ConnectionInfo.filesSendingQueue.contains(uri)) continue;
                ConnectionInfo.filesSendingQueue.add(uri);
            }
        }

        if (!isNotifySendFileRunning)
            notifySendFile(ConnectionInfo.filesSendingQueue);
        else{
            if(currQueueSize < ConnectionInfo.filesSendingQueue.size()){
                toastOnUI("已加入传输队列");
            }
        }

    }

    static volatile boolean isNotifySendFileRunning = false;
    /*
     * @param:file_paths 待发送的文件名数组
     * @desc: 每发送一个文件信息给PC端后空转阻塞（PC端会告知Android端以收到，Android端会发起TCP文件传输连接至PC端，然后发送文件），待文件发送完成后再发送下一个文件信息
     * */
    public void notifySendFile(final CopyOnWriteArrayList<Uri> uris) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                isNotifySendFileRunning = true;
                //显示进度对话框
                FileProgressDialogThread fileProgressDialogThread = new FileProgressDialogThread(null ,uris);
                fileProgressDialogThread.start();

                fileLoop:
                for (int i = 0; i < uris.size(); i++) {
                    Uri uri = uris.get(i);
                    File dest = new File(MainActivity.this.getCacheDir(), utils.getFileNameFromUri(uri.toString()));
                    try {
                        if (!utils.copyFileUsingStream(getContentResolver().openInputStream(uri), dest))
                            continue ;
                    } catch (IOException e) {
                        e.printStackTrace();
                        continue ;
                    }
                    final String file_path = dest.getAbsolutePath();
                    System.out.println("文件路径：" + file_path);
                    ConnectionInfo.sendFileName = file_path;
                    if (!dest.exists()) {
                        toastOnUI("文件不存在:" + file_path);
                        continue;
                    }


                    System.out.println("==========第" + (i + 1) + "个文件开始发送==========");
                    System.out.println("文件路径：" + file_path);
                    toastOnUI("开始发送文件：" + dest.getName());

                    //向PC端发送 "文件名"和"文件大小（字节）",如果有回应则发送文件
                    writeChannelThread.send(ConnectionInfo.FILEINFOCONTROLMESSAGE + utils.getFileInfo(dest));
                    //忙等待本文件发完
                    while (!ConnectionInfo.filesSendedSet.contains(file_path)) {
                        try {
                            fileProgressDialogThread.setProgress(sendFileConnection == null ? 0 : sendFileConnection.progress);
                            Thread.sleep(100);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        } finally {
                            //如果传输过程中连接断开，则清除发送文件队列，并退出此方法
                            if (!tcpConnectionChannel.establishFlag || fileTransferProgressExitFlag) {
                                toastOnUI(fileTransferProgressExitFlag ? "文件传输中止" : "连接异常，传输失败");
                                break fileLoop;
                            }
                        }
                    }
                    //改变进度对话框到下一个文件并重置进度
                    fileProgressDialogThread.next();

                    toastOnUI("文件 " + dest.getName() + " 发送完成");
                    System.out.println("==========第" + (i + 1) + "个文件发送完成==========");
                    dest.delete();
                }

                ConnectionInfo.resetSendInfo();
                setIntent(null);
                isNotifySendFileRunning = false;
            }
        }).start();
    }

    /*
     * 开启子线程接收文件
     * */
    public void receiveFile() {

        try {
            receiveFileConnection = new tcpFileConnectionReceiveChannelThread();
            receiveFileConnection.connectTo();
            if (receiveFileConnection.establishFlag) {
                receiveFileConnection.start();
                //显示进度对话框
                CopyOnWriteArrayList<String> list = new CopyOnWriteArrayList<>();
                list.add(receiveFileInfo.get("fileName"));
                new FileProgressDialogThread(receiveFileConnection, list).start();
            }
        } catch (Exception e) {
            e.printStackTrace();
            receiveFileConnection.closeConnection();
        }


    }

    /*
     * 开启子线程发送filePath指定的文件
     * */
    public void sendFile(final String filePath) {

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    sendFileConnection = new tcpFileConnectionSendChannelThread(filePath);
                    sendFileConnection.connectTo();
                    if (sendFileConnection.establishFlag) {
                        sendFileConnection.start();
                    }
                } catch (IOException e) {
                    toastOnUI("打开文件失败");
                    e.printStackTrace();
                    sendFileConnection.closeConnection();
                } catch (Exception e) {
                    e.printStackTrace();
                    toastOnUI("无法连接至目标主机");
                    sendFileConnection.closeConnection();
                }
            }
        }).start();
    }

    public void fileListChanged(final ArrayList<String> files){
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                noFileTextView.setVisibility(files.size() == 0 ? View.VISIBLE:View.GONE);
                fileListAdapter.dataChange(files);
            }
        });
    }


    public void UIInit() {

        if(this.getApplicationContext().getResources().getConfiguration().uiMode == 0x21){//深色0x21
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                WindowManager.LayoutParams lp = getWindow().getAttributes();
                lp.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
                getWindow().setAttributes(lp);
            }
        }else{ //浅色0x11
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                this.getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
            }
        }


        //两个滑动页面初始化
        LinearLayout.LayoutParams layoutParams = (LinearLayout.LayoutParams) page1.getLayoutParams();
        LinearLayout linearLayout;
        layoutParams.width = widthPixels;// 控件的宽强制设成屏幕宽度
        layoutParams.height = new Double(heightPixels * 190 / 460.0).intValue();
        page1.setLayoutParams(layoutParams); //使设置好的布局参数应用到控件

        LinearLayout.LayoutParams layoutParams2 = (LinearLayout.LayoutParams) page2.getLayoutParams();
        layoutParams2.width = widthPixels;// 控件的宽强制设成屏幕宽度
        layoutParams2.height = new Double(heightPixels * 190 / 460.0).intValue();
        page2.setLayoutParams(layoutParams2); //使设置好的布局参数应用到控件

        pagesScrollView.setHorizontalFadingEdgeEnabled(false);


        //编辑框文字以及checkbox是否勾选
        edit_ip.setText(Config.getConfiguration("serverIp"));
        edit_port.setText(Config.getConfiguration("serverPort"));
        serverIp = Config.getConfiguration("serverIp");
        try {
            serverPort = Integer.parseInt(Config.getConfiguration("serverPort"));
        } catch (NumberFormatException e) {
            serverPort = 12306;
            edit_port.setText("12306");
        }

        checkBox_ifEnableAutoSend.setChecked(Config.getConfiguration("enableAutoSend").contentEquals("true"));
        ifClipChangedSend = checkBox_ifEnableAutoSend.isChecked();
        if (this.checkBox_ifEnableAutoSend.isChecked())
            enableAutoSend(this.checkBox_ifEnableAutoSend);

        secretTextView.setText(Config.getConfiguration("secret"));

        //adapter
        fileListAdapter = new FileListAdapter(MainActivity.this);
        fileListView.setAdapter(fileListAdapter);
        fileListChanged(FileUtils.getFilesAllName(Config.RECEIVEFILEDIRETORY));

        //底部菜单栏的按钮图标
        fileTab.setImageLevel(3);

        fileDialogInit();
    }


    private void setUpListener() {

        dotTouchListener dottouchListener = new dotTouchListener();
        settingIcon.setOnTouchListener(dottouchListener);

        tabBtnListener tabbtnListener = new tabBtnListener();
        homeTab.setOnTouchListener(tabbtnListener);
        fileTab.setOnTouchListener(tabbtnListener);

        pagesListener = new touchOnPagesListener();
        pagesScrollView.setOnTouchListener(pagesListener);
        editTextEditListener editListener = new editTextEditListener();
        edit_ip.addTextChangedListener(editListener);
        edit_port.addTextChangedListener(editListener);


        mRefreshLayout.setOnRefreshListener(
                new CircleRefreshLayout.OnCircleRefreshListener() {
                    @Override
                    public void refreshing() {
                        // do something when refresh starts
                        new Thread(new Runnable() {
                            @Override
                            public void run() {

                                fileListChanged(FileUtils.getFilesAllName(Config.RECEIVEFILEDIRETORY));
                                try {
                                    Thread.sleep(1200);
                                } catch (Exception e) {
                                } finally {
                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            mRefreshLayout.finishRefreshing();
                                            mRefreshLayout.backtotop();//转移到主线程里做
                                        }
                                    });
                                }
                            }
                        }).start();
                    }

                    @Override
                    public void completeRefresh() {
                        // do something when refresh complete
                        toastOnUI("刷新成功~");
                    }
                });

        fileListView.setOnItemClickListener(new fileListViewOnTouchListener());


    }


    public void runnableInit() {

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
                        uiSetIpPort(udpServer.ip, udpServer.port);
                        System.out.println("已自动识别并设置IP与端口号");
                    }
                });
            }
        };

        listViewRunnable = new Runnable() {
            @Override
            public void run() {
               fileListChanged(FileUtils.getFilesAllName(Config.RECEIVEFILEDIRETORY));
            }
        };
    }

    //初始化右上角菜单点击“剪贴记录”后的弹窗
    AlertDialog historyAlertDialog;
    View view;
    ListView hisListView;
    ArrayAdapter<String> adapter;
    public static Handler handler;

    public void popup_menu_window_init() {
        handler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                adapter.notifyDataSetChanged();
            }
        };
        view = getLayoutInflater().inflate(R.layout.popup_menu_window, null);

        historyAlertDialog = new AlertDialog.Builder(MainActivity.this)
                .setTitle("剪贴板历史记录")
                .setIcon(R.drawable.clip)
                .setView(view)
                .setPositiveButton("确定", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface paramAnonymousDialogInterface,
                                        int paramAnonymousInt) {
                    }
                }).create();
        historyAlertDialog.getWindow().setBackgroundDrawable(new ColorDrawable(getResources().getColor(R.color.bgColor)));
        hisListView = view.findViewById(R.id.hisListView);
        adapter = new ArrayAdapter<>(MainActivity.this, android.R.layout.simple_list_item_1, clipDatas);

        hisListView.setAdapter(adapter);
        hisListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                disableClipChangeSend();
                toastOnUI("复制成功~");
                MainActivity.clipboard.setText(clipDatas.get(position));
                enableClipChangeSend();
            }
        });

    }

    public View fileTransferProgressView;
    public AlertDialog fileTransferProgressDialog;
    public FlikerProgressBar transfer_progress_bar;
    public TextView sendStatusTextView;

    public void fileDialogInit() {
        fileTransferProgressView = getLayoutInflater().inflate(R.layout.file_transfer_progress, null);
        transfer_progress_bar = fileTransferProgressView.findViewById(R.id.transfer_progress_bar);
        sendStatusTextView = fileTransferProgressView.findViewById(R.id.sendStatusTextView);
        fileTransferProgressDialog = new AlertDialog.Builder(MainActivity.this)
                .setTitle("传输文件")
                .setIcon(R.drawable.pigeon)
                .setView(fileTransferProgressView)
                .setCancelable(false)
                .setPositiveButton("取消传输", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface paramAnonymousDialogInterface,
                                        int paramAnonymousInt) {
                        //取消传输
                        fileTransferProgressExitFlag = true;
                        if (sendFileConnection != null){
                            sendFileConnection.closeSocket();
                            sendFileConnection.closeConnection();
                        }
                        if (receiveFileConnection != null){
                            receiveFileConnection.closeSocket();
                            receiveFileConnection.closeConnection();
                        }
                    }
                }).create();
        fileTransferProgressDialog.getWindow().setBackgroundDrawable(new ColorDrawable(getResources().getColor(R.color.bgColor)));

    }


    //页面底部的两个tabbar图标点击事件监听器
    class tabBtnListener implements View.OnTouchListener {
        @Override
        public boolean onTouch(View v, MotionEvent event) {

            if (v.getId() == homeTab.getId()) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        //按钮图标变换，小图标
                        changeHomeTabSize(-1);
                        break;
                    case MotionEvent.ACTION_UP:
                        //按钮图标变换,正常图标
                        homeTab.setImageLevel(0);
                        fileTab.setImageLevel(3);
                        pagesScrollView.smoothScrollTo(0, 0);
                        changeHomeTabSize(1);
                        break;
                    case MotionEvent.ACTION_CANCEL:
                        homeTab.setImageLevel(1);
                        changeHomeTabSize(1);
                        break;
                }


            } else if (v.getId() == fileTab.getId()) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        //按钮图标变换，小图标
                        changeFileTabSize(-1);
                        break;
                    case MotionEvent.ACTION_UP:
                        //按钮图标变换,正常图标
                        //按钮图标变换
                        homeTab.setImageLevel(1);
                        fileTab.setImageLevel(2);
                        pagesScrollView.smoothScrollTo(widthPixels, 0);
                        changeFileTabSize(1);
                        break;
                    case MotionEvent.ACTION_CANCEL:
                        fileTab.setImageLevel(3);
                        changeFileTabSize(1);
                        break;
                }


            }
            return true;
        }

        //flag为1表示增大图标尺寸，-1反之
        private void changeHomeTabSize(int flag) {
            ViewGroup.LayoutParams sizeParamHome = homeTab.getLayoutParams();
            sizeParamHome.height = sizeParamHome.height + 10 * flag;
            sizeParamHome.width = sizeParamHome.width + 10 * flag;
            homeTab.setLayoutParams(sizeParamHome);
        }

        private void changeFileTabSize(int flag) {
            ViewGroup.LayoutParams sizeParamFile = fileTab.getLayoutParams();
            sizeParamFile.height = sizeParamFile.height + 10 * flag;
            sizeParamFile.width = sizeParamFile.width + 10 * flag;
            fileTab.setLayoutParams(sizeParamFile);
        }

    }

    //右上角设置的点击监听器
    class dotTouchListener implements View.OnTouchListener {

        @SuppressLint("ClickableViewAccessibility")
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            // TODO Auto-generated method stub
            popupMenu.showLocation(R.id.more_dots);// 设置弹出菜单弹出的位置
            // 设置回调监听，获取点击事件
            popupMenu.setOnItemClickListener(new PopupMenu.OnItemClickListener() {

                @Override
                public void onClick(PopupMenu.MENUITEM item, String str) {
                    // TODO Auto-generated method stub
                    System.out.println("点击：" + item + "  " + str);
                    if (PopupMenu.MENUITEM.RECONNECT == item) {
                        DetectThread.detectFlag = false;
                        DetectThread.interrupted();
                        closeConnection();
                        DetectThread.detectFlag = true;
                        detectThread = new DetectThread(MainActivity.this);
                        detectThread.start();
                        toastOnUI("重置成功~");
                    } else if (PopupMenu.MENUITEM.CLIPHISTORY == item) {
                        handler.sendEmptyMessageDelayed(100, 1000);
                        historyAlertDialog.show();
                        UIUtils.setDialogTitleColor(historyAlertDialog, getResources().getColor(R.color.textColor));
                    } else if (PopupMenu.MENUITEM.DETELEALL == item) {

                        new android.app.AlertDialog.Builder(MainActivity.this)
                                .setTitle("删除文件")
                                .setIcon(R.drawable.delete)
                                .setMessage("确定要删除所有接收到的文件吗？")
                                .setPositiveButton("确定", new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface paramAnonymousDialogInterface,
                                                        int paramAnonymousInt) {
                                        toastOnUI(FileUtils.deleteAllFileInDir(Config.RECEIVEFILEDIRETORY) ? "删除成功~" : "删除失败，请检查是否授予权限");
                                        new Thread(MainActivity.listViewRunnable).start();
                                    }
                                })
                                .setNegativeButton("取消", null)
                                .create()
                                .show();


                    }else if (PopupMenu.MENUITEM.SPECIFICATION == item) {
                        AlertDialog alertDialog = new AlertDialog.Builder(MainActivity.this)
                                .setTitle(getString(R.string.useSpecificationTitle))
                                .setIcon(R.drawable.specification)
                                .setView(R.layout.use_specification)
                                .setPositiveButton("确定", null)
                                .create();
                        alertDialog.getWindow().setBackgroundDrawable(new ColorDrawable(getResources().getColor(R.color.bgColor)));
                        alertDialog.show();
                        UIUtils.setDialogTitleColor(alertDialog, getResources().getColor(R.color.textColor));

                    }

                }
            });
            return true;
        }
    }


    public void setMotionEventX(touchOnPagesListener listener, float x) {
        listener.startXPosition = x;
    }

    //主页面页面切换的滑动监听器
    class touchOnPagesListener implements View.OnTouchListener {

        public float startXPosition;
        public float startYPosition;

        public float endXPosition;

        int currTab = 0;

        @Override
        public boolean onTouch(View v, MotionEvent event) {
//            System.out.println("touch-----------------------------------");
//            System.out.println(event.getX());
//            System.out.println(event.getY());
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    startXPosition = event.getX();
                    startYPosition = event.getY();
                    break;
                case MotionEvent.ACTION_UP:
                    mRefreshLayout.onTouchEvent(event);
                    endXPosition = event.getX();
                    if (startXPosition - endXPosition > widthPixels / 5) {
                        pagesScrollView.smoothScrollTo(widthPixels, 0);
                        currTab = 1;
                        homeTab.setImageLevel(1);
                        fileTab.setImageLevel(2);
                        break;
                    }
                    if (endXPosition - startXPosition > widthPixels / 5) {
                        pagesScrollView.smoothScrollTo(0, 0);
                        currTab = 0;
                        homeTab.setImageLevel(0);
                        fileTab.setImageLevel(3);
                        break;
                    }
                    pagesScrollView.smoothScrollTo(currTab * widthPixels, 0);

                    break;
                case MotionEvent.ACTION_MOVE:
                    if (currTab == 1){
                        if (Math.abs(event.getY() - startYPosition) > Math.abs(event.getX() - startXPosition)){
                            mRefreshLayout.onTouchEvent(event);
                            mRefreshLayout.requestDisallowInterceptTouchEvent(true);
                            return true;
                        }
                    }
                    mRefreshLayout.requestDisallowInterceptTouchEvent(false);
                    return false;
                default:
                    break;
            }
            return true;
        }
    }

    public static boolean fileTransferProgressExitFlag = false;
    public static boolean nextFileFlag = false;
    public static boolean firstTransfer = true;
    class FileProgressDialogThread extends Thread {

        CopyOnWriteArrayList<String> filePaths;
        tcpFileConnectionChannel fileConnectionChannel;
        float progress;

        public FileProgressDialogThread(tcpFileConnectionChannel fileConnectionChannel, CopyOnWriteArrayList uris) {
            this.fileConnectionChannel = fileConnectionChannel;
            this.filePaths = new CopyOnWriteArrayList<>();
            for (Object uri : uris) {
                this.filePaths.add(uri.toString());
            }
        }


        public void next() {
            nextFileFlag = true;
        }

        public void setProgress(float progress) {
            this.progress = progress;
        }

        @Override
        public void run() {
            fileTransferProgressExitFlag = false;
            System.out.println("显示文件传输对话框");
            System.out.println("filePaths.size: " + filePaths.size());
            for (String path: filePaths) {
                System.out.println("传输文件：" + path);
            }
            for (int i = 0; i < filePaths.size(); i++) {
                @SuppressLint("DefaultLocale")
                final String title = String.format("当前第%d个，总共%d个", i+1, filePaths.size());
                final String msg = utils.getFileNameFromUri(filePaths.get(i));
                final int idx = i;

                //1.创建传输进度对话框
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        fileTransferProgressDialog.setTitle(title);
                        fileTransferProgressDialog.show();
                        UIUtils.setDialogTitleColor(fileTransferProgressDialog, getResources().getColor(R.color.textColor));
                        sendStatusTextView.setText("正在传输:" + msg);
                        fileTransferProgressDialog.getButton(DialogInterface.BUTTON_POSITIVE).setText("取消传输");
                        if (!firstTransfer){
                            transfer_progress_bar.reset();
                        }
                        firstTransfer = false;
                    }
                });
                nextFileFlag = false;
                while (tcpConnectionChannel.establishFlag && !nextFileFlag) {
                    try {
                        Thread.sleep(100);
                        //2.如果进度有变化，则更新进度条
                        final float progress = fileConnectionChannel == null ? this.progress : fileConnectionChannel.progress;
                        @SuppressLint("DefaultLocale")
                        final float ui_progress = Float.parseFloat(String.format("%.2f", progress));
                        if (ui_progress != transfer_progress_bar.getProgress())
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    transfer_progress_bar.setProgress(ui_progress);
                                }
                            });
                    } catch (InterruptedException e) {
                        fileTransferProgressDialog.dismiss();
                        nextFileFlag = true;
                        e.printStackTrace();
                        break;
                    } finally {
                        //如果传输过程中连接断开，或传输取消，则清除发送文件队列，并退出此方法
                        if (!tcpConnectionChannel.establishFlag || fileTransferProgressExitFlag) {
                            System.out.println("!tcpConnectionChannel.establishFlag:" + !tcpConnectionChannel.establishFlag);
                            System.out.println("fileTransferProgressExitFlag:" + fileTransferProgressExitFlag);
                            fileTransferProgressDialog.dismiss();
                            return;
                        }
                    }
                }
                System.out.println("tcpConnectionChannel.establishFlag: " + tcpConnectionChannel.establishFlag + "      "  + "!nextFileFlag: " + !nextFileFlag);

                //3.结束进度条
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        sendStatusTextView.setText("文件传输完成");
                        fileTransferProgressDialog.getButton(DialogInterface.BUTTON_POSITIVE).setText("确定");
                        transfer_progress_bar.setProgress(100);
                        transfer_progress_bar.finishLoad();
                    }
                });


            }
            System.out.println("文件传输进度界面线程退出");
        }
    }

    //IP、端口号编辑监听器
    class editTextEditListener implements TextWatcher {


        @Override
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before, int count) {
        }

        @Override
        public void afterTextChanged(Editable s) {
                if (!"".contentEquals(edit_port.getText().toString()) && !"".contentEquals(edit_ip.getText().toString())) {
                    //从输入框得到IP和端口
                    serverIp = edit_ip.getText().toString();
                    try {
                        serverPort = Integer.parseInt(edit_port.getText().toString());
                    } catch (Exception e) {
                        serverPort = 0;
                    }
                    //保存至文件
                    saveConfig();
                }
        }
    }

    //为filePath指定的文件打开文件操作对话框
    public void onReceiveFileClick(String filePath) {
        File file = new File(filePath);
        new bottomFileOpDialog(file, this).onDialog();
    }

    //接收到的文件列表的item点击事件监听器
    class fileListViewOnTouchListener implements AdapterView.OnItemClickListener {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            onReceiveFileClick((String) parent.getItemAtPosition(position));
        }
    }


    //每次打开页面检测剪贴板有无变化的检测线程
    public static String preClipBoardText = "";

    private void checkClipBoardText() {
        final Context self = this;
        // 添加剪贴板数据改变监听器(用线程每秒检测来实现全局监听。因为剪切板只会被当前获取焦点的应用得到
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    Thread.sleep(1000);
                } catch (Exception e) {
                }
                // 勾选了界面的剪贴板检测、且没有被其他模块屏蔽（if_shield_clip_detect 不为true时）才会发送
                if (ifClipChangedSend) {
                    if (clipboard.getText() != null)
                        if (!preClipBoardText.contentEquals(clipboard.getText())) {
                            if (!if_shield_clip_detect) {
                                sendMessage(getSystemInfo.getClipboardContent(self));
                                clipDatas.addFirst(clipboard.getText().toString());
                                handler.sendEmptyMessageDelayed(100, 0);
                                System.out.println("检测到剪贴板文本变化，发送至PC");
                            }
                            preClipBoardText = clipboard.getText().toString();
                        }
                }
            }
        }).start();
    }

}



