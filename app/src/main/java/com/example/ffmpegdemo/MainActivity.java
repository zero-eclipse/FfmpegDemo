package com.example.ffmpegdemo;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import com.example.ffmpegdemo.util.FFmpegUtil;

import org.libsdl.app.SDLActivity;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity {




    private TextView textView;


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
        FFmpegUtil util = new FFmpegUtil(this);
        textView.setText(util.FFmpegMov2YUV());
    }
}

