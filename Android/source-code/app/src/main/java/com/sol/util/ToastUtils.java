package com.sol.util;

import android.content.Context;
import android.widget.Toast;

public class ToastUtils {
    public static void showToast(Context context, String msg){
        Toast.makeText(context, msg, Toast.LENGTH_SHORT).show();
    }
    public static void showToast(Context context, int stringID){
        showToast(context, context.getString(stringID));
    }
}
