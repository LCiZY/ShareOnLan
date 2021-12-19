package com.sol.adapter;

import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.content.Context;
import android.view.LayoutInflater;
import android.widget.ImageView;
import android.widget.TextView;

import com.sol.R;
import com.sol.util.FileUtils;
import com.sol.util.utils;

import java.io.File;
import java.util.ArrayList;

public class FileListAdapter extends BaseAdapter {

    private ArrayList<String> files;
    private LayoutInflater layoutInflater;

    public FileListAdapter(Context context, ArrayList<String> files) {
        this.files = files;
        this.layoutInflater = LayoutInflater.from(context);

    }

    public void dataChange(ArrayList<String> files) {
        this.files = files;
        this.notifyDataSetChanged();
    }


    @Override
    public int getCount() {
        return files.size();
    }

    @Override
    public Object getItem(int position) {
        if (position < files.size())
            return files.get(position);
        return null;
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    static class ViewHolder {
        public ImageView l_image;
        public TextView l_title, l_time;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder viewHolder = null;
        if (convertView == null) {
            convertView = layoutInflater.inflate(R.layout.layout_list_item, null);
            viewHolder = new ViewHolder();
            viewHolder.l_image = convertView.findViewById(R.id.itemImage);
            viewHolder.l_title = convertView.findViewById(R.id.itemTitle);
            viewHolder.l_time = convertView.findViewById(R.id.itemTime);
            convertView.setTag(viewHolder);
        } else {
            viewHolder = (ViewHolder) convertView.getTag();
        }

        //System.out.println(data.size());
        if (files.size() > 0 && files.size() > position) {
            File file = new File(files.get(position));
            viewHolder.l_image.setImageLevel(getFileItemIconLevel(FileUtils.getFileSuffix(file)));
            viewHolder.l_title.setText(file.getName());
            viewHolder.l_time.setText(utils.longToDate(file.lastModified()));
        }


        return convertView;
    }


    public int getFileItemIconLevel(final String suffix) {
        final String suffix_lower_case = suffix.toLowerCase();
        switch (suffix_lower_case) {
            case "docx":
            case "doc":
                return 1;
            case "xlsx":
            case "xls":
                return 2;
            case "pptx":
            case "ppt":
            case "pps":
                return 3;
            case "pdf":
                return 4;
            case "zip":
            case "7z":
            case "gz":
            case "rar":
            case "z":
            case "arj":
                return 5;
            case "txt":
            case "log":
                return 6;
            case "cpp":
            case "c":
            case "py":
            case "java":
            case "class":
            case "jsp":
            case "html":
            case "css":
            case "js":
            case "go":
            case "php":
                return 7;
            case "jpeg":
            case "gif":
            case "png":
            case "jpg":
            case "bmp":
            case "psd":
            case "ai":
            case "tiff":
            case "eps":
            case "svg":
            case "cr2":
            case "nef":
            case "dng":
                return 8;
            case "mp4":
            case "mpg":
            case "mpeg":
            case "avi":
            case "rm":
            case "rmvb":
            case "mov":
            case "wmv":
            case "asf":
            case "dat":
                return 9;
            case "mp3":
            case "cda":
            case "wav":
            case "aif":
            case "aiff":
            case "mid":
            case "ra":
            case "wma":
            case "vqf":
            case "ape":
            case "flac":
                return 10;
            case "apk":
                return 11;

            default:
                return 0;
        }
    }


}
