package com.sol;

import android.util.Base64;

import com.sol.util.CheckArgumentUtil;
import com.sol.util.utils;

import org.junit.Test;

import java.util.HashMap;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
public class ExampleUnitTest {
    @Test
    public void addition_isCorrect() {

        String ip = "92.255.1.0";
        //匹配  数字.数字.数字.数字 的形式
        String reg = "((([12][0-9][0-9])|([1-9][0-9])|([0-9]))\\.){3}(([12][0-9][0-9])|([1-9][0-9])|([0-9]))";
        System.out.println(CheckArgumentUtil.checkIfIpValidate(ip));

    }

    @Test
    public void test2(){
        String line = "dada= ";
        String[] lineData = line.split("=");
        System.out.println(lineData[0]);
        System.out.println(lineData[1]);
    }


    @Test
    public void test3(){

        HashMap<String,String> configurations = new HashMap<>();
        configurations.put("1","a");
        configurations.put("3","c");
        configurations.put("2","b");
        configurations.put("4","d");
        String ini = "";
        for (String key: configurations.keySet() ){
            ini+=(key+"="+configurations.get(key)+"\n");
        }
        System.out.println(ini);
    }

    private static final String ALGORITHM = "AES";
    private static final String AES_ECB_PADDING = "AES/ECB/PKCS7Padding";//AES/ECB/PKCS7Padding

    /**
     * Aes加密(ECB工作模式),不要IV
     *
     * @param key  密钥,key长度必须大于等于 3*8 = 24,并且是8的倍数
     * @param data 明文
     * @return 密文
     * @throws Exception
     */
    public static byte[] encodeByECB(byte[] key, byte[] data) throws Exception {
        //获取SecretKey对象,也可以使用getSecretKey()方法
        SecretKey secretKey = new SecretKeySpec(key, ALGORITHM);
        //获取指定转换的密码对象Cipher（参数：算法/工作模式/填充模式）
        Cipher cipher = Cipher.getInstance(AES_ECB_PADDING);
        //用密钥和一组算法参数规范初始化此Cipher对象（加密模式）
        cipher.init(Cipher.ENCRYPT_MODE, secretKey);
        //执行加密操作
        return cipher.doFinal(data);
    }

    /**
     * Aes解密(ECB工作模式),不要IV
     *
     * @param key  密钥,key长度必须大于等于 3*8 = 24,并且是8的倍数
     * @param data 密文
     * @return 明文
     * @throws Exception
     */
    public static byte[] decodeByECB(byte[] key, byte[] data) throws Exception {
        //获取SecretKey对象,也可以使用getSecretKey()方法
        SecretKey secretKey = new SecretKeySpec(key, ALGORITHM);
        //获取指定转换的密码对象Cipher（参数：算法/工作模式/填充模式）
        Cipher cipher = Cipher.getInstance(AES_ECB_PADDING);
        //用密钥和一组算法参数规范初始化此Cipher对象（加密模式）
        cipher.init(Cipher.DECRYPT_MODE, secretKey);
        //执行加密操作
        return cipher.doFinal(data);
    }


    @Test
    public void testAES() throws Exception{
        String text = "123456";
        String secret = "123456";
        String key = utils.bytesToHex(utils.md5(secret));

        byte[] text_bytes = text.getBytes("UTF-8");
        byte[] pass_bytes = key.getBytes("UTF-8");

        System.out.println("key:" + key);

        byte[] encode = encodeByECB(pass_bytes, text_bytes);
        System.out.println(Base64.encodeToString(encode, Base64.NO_WRAP));
//        // strings encryption
//                String encrypted = Aes256.encrypt(text, key);
//                System.out.println(encrypted);
//
//        // bytes encryption
//                byte[] encrypted_bytes = Aes256.encrypt(text_bytes, pass_bytes);
//                System.out.println(encrypted_bytes);
//
//        // strings decryption
//                String decrypted = Aes256.decrypt(encrypted, key);
//                System.out.println(decrypted);
//
//        // bytes decryption
//                byte[] decrypted_bytes = Aes256.decrypt(encrypted_bytes, pass_bytes);
//                System.out.println(decrypted_bytes);
    }

}