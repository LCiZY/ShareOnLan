package com.sil.util;

public class CheckArgumentUtil {

    public  static  boolean  checkIfIpValidate(String ip){
        if(ip==null) return false;
        //匹配  数字.数字.数字.数字 的形式
        String reg = "((([12][0-9][0-9])|([1-9][0-9])|([0-9]))\\.){3}(([12][0-9][0-9])|([1-9][0-9])|([0-9]))";
        if(!ip.matches(reg)) return false;
        String[] ip_splices = ip.split("\\.");
        //判断每个数字大小是否超过255
        for (String ip_splice:ip_splices) {
            if(Integer.parseInt(ip_splice)>255) return false;
        }

         return  true;
    }



}
