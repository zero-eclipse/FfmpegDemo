package com.example.ffmpegdemo.util;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.File;

public class FFmpegUtil {

    static {
        System.loadLibrary("native-lib");
    }


    public  InterFFmpegResult result;


    private Context context;

    public FFmpegUtil(Context context){
        this.context=context;
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String FFmpegMov2YUV();



    public String getMovPath(){
        String basePath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String movPath=basePath+ File.separator+"ffmpeg"+File.separator+"shoe.mp4";

        Log.e("zero","文件是否存在："+new File(movPath).exists());
        return movPath;
    }


    public void call_back(int code){

        if(result!=null){
            result.translateYUV();
        }
    }


}
