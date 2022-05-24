package com.sol.util;

import android.content.Context;
import android.widget.Toast;
import static com.sol.MainActivity.handler;
public class ToastUtils {
    public static void showToast(final Context context, final String msg){
        handler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
            }
        });
    }
    public static void showToast(final Context context, final int stringID){
        handler.post(new Runnable() {
            @Override
            public void run() {
                showToast(context, context.getString(stringID));
            }
        });
    }
}
