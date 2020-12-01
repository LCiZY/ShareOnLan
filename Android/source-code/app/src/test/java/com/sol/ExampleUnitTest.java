package com.sol;

import com.sol.util.CheckArgumentUtil;

import org.junit.Test;


import java.util.HashMap;

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

}