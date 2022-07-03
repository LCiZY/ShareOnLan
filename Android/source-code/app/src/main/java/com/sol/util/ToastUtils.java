package com.sol.util;

import android.content.Context;
import android.os.Handler;
import android.widget.Toast;

public class ToastUtils {
    public static Handler mainHandler;
    public static void showToast(final Context context, final String msg){
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
            }
        });
    }
    public static void showToast(final Context context, final int stringID){
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                showToast(context, context.getString(stringID));
            }
        });
    }
}
