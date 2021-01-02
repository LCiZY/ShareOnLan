package com.sol.component;

import android.app.Dialog;
import android.content.Context;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.view.animation.TranslateAnimation;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.sol.MainActivity;
import com.sol.R;
import com.sol.util.FileUtils;

import java.io.File;
import java.io.IOException;

public class bottomFileOpDialog {

    private File file;
    Context context;
    public bottomFileOpDialog(File file,Context context) {
        this.file = file;
        this.context = context;
    }

    private TextView bottomDialogFileNameTextView;
    private ImageButton deleteFileBtn;
    private LinearLayout openLayout;
    private LinearLayout shareLayout;
    public Dialog dialog;

    public  void onDialog()
    {
        dialog=new Dialog(context);//可以在style中设定dialog的样式
        dialog.setContentView(R.layout.fileop_bottom_dialog);

        bottomDialogFileNameTextView = dialog.findViewById(R.id.bottomDialogFileNameTextView);
        deleteFileBtn = dialog.findViewById(R.id.deleteBtn);
        openLayout = dialog.findViewById(R.id.open);
        shareLayout = dialog.findViewById(R.id.share);

        bottomDialogFileNameTextView.setText(file.getName());
        deleteFileBtn.setOnTouchListener(new deleteFileTabListener());
        openLayout.setOnTouchListener(new openFileTabListener());
        shareLayout.setOnTouchListener(new shareFileTabListener());

        WindowManager.LayoutParams lp=dialog.getWindow().getAttributes();
        lp.gravity= Gravity.BOTTOM;
        lp.height= WindowManager.LayoutParams.WRAP_CONTENT;
        lp.width= WindowManager.LayoutParams.MATCH_PARENT;
        dialog.getWindow().setAttributes(lp);
        //设置该属性，dialog可以铺满屏幕
        dialog.getWindow().setBackgroundDrawable(null);
        dialog.show();
        //      dialog.getWindow().setWindowAnimations();
        slideToUp(dialog.getWindow().findViewById(R.id.layout));
    }

    /**
     * 弹出动画
     * @param view
     */
    private static void slideToUp(View view){
        int druaTime = 600;

        Animation slide = new TranslateAnimation(Animation.RELATIVE_TO_SELF, 0.0f,
                Animation.RELATIVE_TO_SELF, 0.0f, Animation.RELATIVE_TO_SELF,
                1.0f, Animation.RELATIVE_TO_SELF, 0.0f);

        slide.setDuration(druaTime);
        slide.setFillAfter(true);
        slide.setFillEnabled(true);


        view.startAnimation(slide);



        slide.setAnimationListener(new Animation.AnimationListener() {
            @Override
            public void onAnimationStart(Animation animation) {

            }

            @Override
            public void onAnimationEnd(Animation animation) {

            }

            @Override
            public void onAnimationRepeat(Animation animation) {

            }
        });

    }


    class deleteFileTabListener implements View.OnTouchListener{

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            //调用其他应用打开文件
            try{
                final String filename = file.getName();
                if(file.delete()){ //删除成功
                    MainActivity.instance.toastOnUI("删除成功~");
                    new Thread(MainActivity.listViewRunnable).start();
                    dialog.dismiss();
                    return true;
                }
            }catch (Exception e){

            }
            //删除失败
            MainActivity.instance.toastOnUI("文件删除失败");
            return true;
        }
    }


    class openFileTabListener implements View.OnTouchListener{

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            //调用其他应用打开文件
            FileUtils.openFile(context,file);
            dialog.dismiss();
            return true;
        }
    }

    class shareFileTabListener implements View.OnTouchListener{

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            //分享至其他应用
            FileUtils.shareFile(context,file);
            dialog.dismiss();
            return true;
        }
    }


}
