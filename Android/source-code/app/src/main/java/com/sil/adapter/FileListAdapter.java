package com.sil.adapter;

import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.content.Context;
import android.view.LayoutInflater;
import android.widget.ImageView;
import android.widget.TextView;

import com.sil.R;
import com.sil.util.utils;

import java.io.File;
import java.util.ArrayList;

public class FileListAdapter extends BaseAdapter {

    private ArrayList<String> files;
    private LayoutInflater layoutInflater;

    public FileListAdapter(Context context, ArrayList<String> files){
        this.files = files;
        this.layoutInflater=LayoutInflater.from(context);

    }

    public void dataChange( ArrayList<String> files){
        this.files = files;
        this.notifyDataSetChanged();
    }


    @Override
    public int getCount() {
        return files.size();
    }

    @Override
    public Object getItem(int position) {
        if(position<files.size())
        return files.get(position);
        return null;
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    static class ViewHolder{
        public ImageView l_image;
        public TextView l_title,l_time;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder viewHolder=null;
        if(convertView==null){
            convertView=layoutInflater.inflate(R.layout.layout_list_item,null);
            viewHolder=new ViewHolder();
            viewHolder.l_image=convertView.findViewById(R.id.itemImage);
            viewHolder.l_title=convertView.findViewById(R.id.itemTitle);
            viewHolder.l_time=convertView.findViewById(R.id.itemTime);
            convertView.setTag(viewHolder);
        }else{

            viewHolder=(ViewHolder) convertView.getTag();
        }

        //System.out.println(data.size());
        if(files.size()>0&&files.size()>position){
            File file = new File(files.get(position));
            viewHolder.l_title.setText(file.getName());
            viewHolder.l_time.setText(utils.longToDate(file.lastModified()));
        }


        return convertView;
    }
}
