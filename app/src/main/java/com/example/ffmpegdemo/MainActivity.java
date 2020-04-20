package com.example.ffmpegdemo;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.example.ffmpegdemo.util.FFmpegUtil;
import com.example.ffmpegdemo.util.InterFFmpegResult;

import org.libsdl.app.SDLActivity;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity {




    private TextView textView;
    private FFmpegUtil util;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        init();
        // 获取权限
        if(ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                !=PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(this, new String[]{
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.READ_EXTERNAL_STORAGE,
            },10001);
        }else{
            initSet();
        }


    }

    public void init(){
        textView =  findViewById(R.id.sample_text);
    }

    public void initSet(){
        util = new FFmpegUtil(this);
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if(requestCode==10001){
            initSet();
        }
    }

    public void SDL_Render(View view) {
        startActivity(new Intent(this, SDLActivity.class));

    }

    public void Video_Info(View view) {
        textView.setText(util.FFmpegMovInfo());
    }

    public void Video2YUV(final View view) {
        view.setEnabled(false);
        util.setOnCallBackListener(new InterFFmpegResult() {
            @Override
            public void translateYUV() {
                Toast.makeText(MainActivity.this, "yuv转码完成", Toast.LENGTH_SHORT).show();
                view.setEnabled(true);
            }
        });
        util.FFmpegMov2YUV();

    }
}

