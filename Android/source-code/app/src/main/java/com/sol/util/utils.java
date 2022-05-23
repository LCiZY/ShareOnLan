package com.sol.util;


import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.SimpleDateFormat;
import java.util.Date;

public class utils {

    public static boolean isEmpty(String s){
        return s == null || "".contentEquals(s);
    }

    public static String getFileInfo(File file){
        return "|fileName|"+file.getName()+"|fileSize|"+file.length();
    }

    public static String getFileNameFromUri(String uri){
        if (isEmpty(uri)) return "";
        uri = decode(uri);
        int idx = uri.lastIndexOf(File.separator);
        if (idx == uri.length()-1) return decode(uri);
        if (idx != -1) return decode(uri.substring(idx+1));
        return decode(uri);
    }

    public static String decode(String url){
        try {
            String prevURL="";
            String decodeURL=url;
            while(!prevURL.equals(decodeURL))
            {
                prevURL=decodeURL;
                decodeURL= URLDecoder.decode( decodeURL, "UTF-8" );
            }
            return decodeURL;
        } catch (UnsupportedEncodingException e) {
            return "Issue while decoding" +e.getMessage();
        }
    }

    public static boolean copyFileUsingStream(InputStream source, File dest) throws IOException {
        InputStream is = source;
        OutputStream os = null;
        try {
            os = new FileOutputStream(dest);
            byte[] buffer = new byte[1024];
            int length;
            while ((length = is.read(buffer)) > 0) {
                os.write(buffer, 0, length);
            }
        } catch (Exception e){
            e.printStackTrace();
            return false;
        }finally {
            is.close();
            os.close();
        }
        return true;
    }


    public static byte[] getBytes(char[] chars) {
        Charset cs = Charset.forName("UTF-8");
        CharBuffer cb = CharBuffer.allocate(chars.length);
        cb.put(chars);
        cb.flip();
        ByteBuffer bb = cs.encode(cb);
        return bb.array();
    }

    public static byte[] getActualSizeBytes(byte[] bytes){
        if(bytes==null) return null;
        int index = bytes.length-1;
        for (int i=bytes.length-1;i>=0;i--)
            if(bytes[i]==0) {index = i; break;}
        byte[] newBytes=new byte[index+1];
       // Arrays.copyOf(bytes,index+1);
        for (int i=0;i<newBytes.length;i++)
            newBytes[i] = bytes[i];
        return newBytes;
    }

    public static String longToDate(long time){
        Date date = new Date(time);
        SimpleDateFormat sd = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        return sd.format(date);
    }


    /**
     * 使用SHA1算法对字符串进行加密
     * @param str
     * @return
     */
    public static String sha1Encrypt(String str) {

        if (str == null || str.length() == 0) {
            return null;
        }

        char hexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                'a', 'b', 'c', 'd', 'e', 'f' };

        try {

            MessageDigest mdTemp = MessageDigest.getInstance("SHA-1");
            mdTemp.update(str.getBytes("UTF-8"));

            byte[] md = mdTemp.digest();
            int j = md.length;
            char buf[] = new char[j * 2];
            int k = 0;

            for (int i = 0; i < j; i++) {
                byte byte0 = md[i];
                buf[k++] = hexDigits[byte0 >>> 4 & 0xf];
                buf[k++] = hexDigits[byte0 & 0xf];
            }

            return new String(buf);

        } catch (Exception e) {
            return null;
        }
    }

    public static byte[] md5(String str) {
        MessageDigest md5 = null;
        try {
            md5 = MessageDigest.getInstance("MD5");
            md5.update(str.getBytes());
            return md5.digest();//加密
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        return null;
    }

    final static char[] HEX_ARRAY = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    public static String bytesToHex(byte[] bytes) {
        if (bytes == null)
            return "";
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars);
    }


    public static String decrypt(String cipher) {
        if(cipher==null) return "foo foo";
        if(cipher.trim().equals("")) return "foo foo";
        int[] factor = {8,2,7};
        char c1 = cipher.charAt(0);
        char c2 = cipher.charAt(cipher.length()-1);
        char mc = (char)((c1+c2)/2);
        String result = cipher.substring(1,cipher.length()-1);
        char[] temp = result.toCharArray();
        //因为密码是6位，所以最后一个空格是如下位置
        int j = result.length()-7;
        //解密前面的ip和端口号
        for (int i = 0; i <=j; i++) {
            temp[i] = (char)(temp[i] - (i*mc)/temp.length - new Double(Math.pow(-1, i)*factor[i%3]).intValue());
        }
        //解密密文
        for(int i=j+1;i<temp.length;i++){
            temp[i] = (char)(temp[i]+2*i/3);
        }

        return String.valueOf(temp);
    }

    public static String[] objsToStrs(Object[] objects){
        if(objects==null||objects.length==0) return new String[]{};
        String[] strings = new String[objects.length];
        for(int i=0;i<objects.length;i++) {
            strings[i] = (String)objects[i];
        }
        return  strings;
    }


}
