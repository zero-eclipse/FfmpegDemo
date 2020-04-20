package com.example.ffmpegdemo.util;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import java.io.File;

public class FFmpegUtil {

    static {
        System.loadLibrary("native-lib");
    }


    private InterFFmpegResult result;


    private Context context;

    public FFmpegUtil(Context contex){
        this.context=context;
    }

    public void setOnCallBackListener(InterFFmpegResult result){
        this.result=result;
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */


    public native String FFmpegMovInfo();
    public native void FFmpegMov2YUV();



    public String getMovPath(){
        String basePath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String movPath=basePath+ File.separator+"ffmpeg"+File.separator+"shoe.mp4";

        Log.e("zero","文件是否存在："+new File(movPath).exists());
        return movPath;
    }


    // jni任务完成后对java函数的回调
    public void call_back(){

        Log.e("zero","发生回调");

        if(result!=null){
            result.translateYUV();
        }
    }


}
