package com.sil.util;


import java.io.File;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.security.MessageDigest;
import java.text.SimpleDateFormat;
import java.util.Date;

public class utils {


    public static String getFileInfo(File file){

        return " fileName "+file.getName()+" fileSize "+file.length();
    }

    public static byte[] getBytes(char[] chars) {
        Charset cs = Charset.forName("UTF-8");
        CharBuffer cb = CharBuffer.allocate(chars.length);
        cb.put(chars);
        cb.flip();
        ByteBuffer bb = cs.encode(cb);
        return bb.array();
    }

    public static char[] getActualSizeChars(char[] chars,int nowSize){
        if(chars==null) return null;
        if(chars.length==nowSize) return  chars;
        char[] newChar = new char[nowSize];
        for (int i=0;i<nowSize;i++)
            newChar[i] = chars[i];
        return newChar;
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


}
