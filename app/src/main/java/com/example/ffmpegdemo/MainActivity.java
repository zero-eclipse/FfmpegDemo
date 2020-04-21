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

public class MainActivity extends AppCompatActivity implements InterFFmpegResult {


    private TextView textView;
    private FFmpegUtil util;
    private View view;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        init();
        // 获取权限
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                != PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.READ_EXTERNAL_STORAGE,
            }, 10001);
        } else {
            initSet();
        }


    }

    public void init() {
        textView = findViewById(R.id.sample_text);
    }

    public void initSet() {
        util = new FFmpegUtil(this);
        util.setOnCallBackListener(this);
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == 10001) {
            initSet();
        }
    }

    public void SDL_Render(View view) {
        startActivity(new Intent(this, SDLActivity.class));

    }

    public void Video_Info(View view) {
        if (this.view != null) {
            return;
        }
        view.setEnabled(false);
        this.view = view;
        util.FFmpegMovInfo();
    }

    public void Video2YUV(final View view) {
        if (this.view != null) {
            return;
        }
        view.setEnabled(false);
        this.view = view;
        util.FFmpegMov2YUV();
    }

    public void Audio2PCM(View view) {
        if (this.view != null) {
            return;
        }
        view.setEnabled(false);
        this.view = view;
        util.FFmpegAudio();
    }

    // JNI的借口回调(获取视频信息，这里没在将视频同伙借口返回，而是函数返回)
    @Override
    public void obtainInfo() {
        Toast.makeText(MainActivity.this, "获取信息完成", Toast.LENGTH_SHORT).show();
        this.view.setEnabled(true);
        this.view = null;
    }


    // JNI的借口回调(视频转码)
    @Override
    public void translateVideo() {
        Toast.makeText(MainActivity.this, "视频转码yuv完成", Toast.LENGTH_SHORT).show();
        this.view.setEnabled(true);
        this.view = null;

    }

    // JNI的借口回调(音频转码)
    @Override
    public void translateAudio() {
        Toast.makeText(MainActivity.this, "音频转码pcm完成", Toast.LENGTH_SHORT).show();
        this.view.setEnabled(true);
        this.view = null;
    }


}

