package com.sil.util;


import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;


public class getSystemInfo {

    //系统剪贴板-获取:
    public static String getClipboardContent(Context context) {
        // 获取系统剪贴板
        ClipboardManager clipboard = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
        // 返回数据
        ClipData clipData = clipboard.getPrimaryClip();
        if (clipData != null && clipData.getItemCount() > 0) {
            // 从数据集中获取（粘贴）第一条文本数据
            return clipData.getItemAt(0).getText().toString();
        }
        return "";
    }

}
