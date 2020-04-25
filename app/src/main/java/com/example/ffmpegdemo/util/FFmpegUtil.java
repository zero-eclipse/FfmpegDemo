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


    // 获取视频信息
    public native String FFmpegMovInfo();
    // 视频每帧转码YUV420p
    public native void FFmpegMov2YUV();
    // 音频解码
    public native void FFmpegAudio();
    // 视频压缩
    public native void FFmpegCompressMov(String[] argv);



    public String getMovPath(){
        String basePath = Environment.getExternalStorageDirectory().getAbsolutePath();
        String movPath=basePath+ File.separator+"ffmpeg"+File.separator+"shoe.mp4";

        Log.e("zero","文件是否存在："+new File(movPath).exists());
        return movPath;
    }


    // jni任务完成后对java函数的回调
    public void call_back(int code){


        if(result!=null){
            switch (code){
                case 100:
                    result.obtainInfo();
                    break;
                case 200:
                    result.translateVideo();
                    break;
                case 300:
                    result.translateAudio();
                    break;
            }

        }
    }




}
