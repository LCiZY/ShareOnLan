package com.sol.bean;

public  class FileInfo {
    public static final String field_fileName = "fileName";
    public static final String field_fileSize = "fileSize";
    public String fileName;
    public Long fileSize;


    public static FileInfo processReceiveFileInfo(final String info){
        if(info==null) return null;
        FileInfo rt = new FileInfo();
        String[] infos = info.split("\\|");
        for (int i=1;i+1<infos.length;i+=2){
            if (FileInfo.field_fileName.contentEquals(infos[i]))
                rt.fileName = infos[i+1];
            if (FileInfo.field_fileSize.contentEquals(infos[i]))
                rt.fileSize = Long.parseLong(infos[i+1]);
        }
        return  rt;
    }
}