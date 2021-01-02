package com.sol.config;

import android.os.Environment;

import com.sol.MainActivity;
import com.sol.util.utils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;

import java.util.HashMap;

public class Config {

    private static final HashMap<String,String> configurations = new HashMap<>();

    public static  String ROOTDIROTERYNAME;
    public static  String RECEIVEDIROTERYNAME ;
    public static  String APPROOTDIR ;
    public static  String TODIRRETORY ;
    public static  String CONFIGFILENAME;


    public static  String RECEIVEFILEDIRETORY ;
    public static void conFigInit(){

        varInit();

        initDuringFirstLaunch();

        readConfigFromFile();


    }

    public static void varInit(){
        ROOTDIROTERYNAME = "ShareOnLan";
        RECEIVEDIROTERYNAME = "ReceiveFiles";
        CONFIGFILENAME = "config.ini";
        APPROOTDIR = MainActivity.instance.getExternalFilesDir(null).getPath() + File.separator + ROOTDIROTERYNAME;
        System.out.println("文件路径-------------------------------");
        System.out.println(MainActivity.instance.getExternalFilesDir(null).getPath() );
        TODIRRETORY = APPROOTDIR+File.separator ;
        RECEIVEFILEDIRETORY = TODIRRETORY+"ReceiveFiles"+File.separator;
    }

    public static void enableDefaultConfiguration(){
        configurations.put("serverIp","192.168.0.1");
        configurations.put("serverPort","12306");
        configurations.put("enableAutoSend","false");
        configurations.put("enableSend","true");
        configurations.put("enableReceive","true");
        configurations.put("secret",createDefaultSecret());

    }

    public static String getConfiguration(String config){
        return configurations.get(config);
    }

    public static void setConfiguration(String config,String value){
        if(value==null) value="";
        configurations.put(config,"".contentEquals(value)?"null":value);

    }

    public static void initDuringFirstLaunch(){
        File file = new File(TODIRRETORY+CONFIGFILENAME);
        File dire = new File(RECEIVEFILEDIRETORY);
        try {
            if (!file.exists()) {

                //建立新文件
                System.out.println(TODIRRETORY);
                File dir = new File(TODIRRETORY);
                System.out.println("创建文件："+ dir.mkdirs());

                file.createNewFile();
                FileOutputStream outStream = new FileOutputStream(file);
                outStream.write("".getBytes());
                outStream.close();
                outStream.flush();
                //写入默认配置
                enableDefaultConfiguration();
                writeConfigToFile();
            }
            if(!dire.exists()) dire.mkdir();
        }catch (Exception e){ e.printStackTrace();}

    }

    public  static void readConfigFromFile()  {
        try {
            String fileName = TODIRRETORY + CONFIGFILENAME;
            FileReader fileReader = new FileReader(fileName);
            BufferedReader bufferedReader = new BufferedReader(fileReader);
            String line = bufferedReader.readLine();
            while (line != null && !"".contentEquals(line)) {
                String[] lineData = line.split("=");
                if (lineData != null && lineData.length == 2) //如果属性是null，则配置为""空串
                    configurations.put(lineData[0], lineData[1].contentEquals("null") ? "" : lineData[1]);

                line = bufferedReader.readLine();
                if (line != null) line = line.trim();
                System.out.println("line:"+line);
            }
        }
        catch (Exception e){
            e.printStackTrace();
            enableDefaultConfiguration();
        }


    }


    public static boolean writeConfigToFile() {
        boolean ok=false;
        String ini = "";
        String filePath = TODIRRETORY + CONFIGFILENAME;
        try {
            File file = new File(filePath);
            FileOutputStream outStream = new FileOutputStream(file);
            for (String key: configurations.keySet() ){
                ini+=(key+"="+configurations.get(key)+"\n");
            }
       //     System.out.println("写入配置文件内容："+ini);

            if(null!=ini&&!"".contentEquals(ini)){
                outStream.write(ini.getBytes());
                ok=true;
            }
            outStream.close();
            outStream.flush();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return ok;
    }



    public static String createDefaultSecret(){
        Object o = new Object();
        String secret = utils.sha1Encrypt(o.hashCode()+""+System.currentTimeMillis()).substring(0,6);
        return secret;
    }


}
