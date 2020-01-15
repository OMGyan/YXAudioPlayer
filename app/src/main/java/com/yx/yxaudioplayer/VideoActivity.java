package com.yx.yxaudioplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Environment;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import com.yx.yxaudioplayerlib.YXTimeInfoBean;
import com.yx.yxaudioplayerlib.listener.yxOnCompleteListener;
import com.yx.yxaudioplayerlib.listener.yxOnErrorListener;
import com.yx.yxaudioplayerlib.listener.yxOnLoadListener;
import com.yx.yxaudioplayerlib.listener.yxOnPauseResumeListener;
import com.yx.yxaudioplayerlib.listener.yxOnPcmInfoListener;
import com.yx.yxaudioplayerlib.listener.yxOnPreparedListener;
import com.yx.yxaudioplayerlib.listener.yxOnTimeInfoListener;
import com.yx.yxaudioplayerlib.log.MyLog;
import com.yx.yxaudioplayerlib.opengl.YXGLSurfaceView;
import com.yx.yxaudioplayerlib.player.YXAudioPlayer;

import java.io.File;

public class VideoActivity extends AppCompatActivity {

    //读写权限
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE};
    //请求状态码
    private static int REQUEST_PERMISSION_CODE = 1;

    private YXGLSurfaceView surfaceView;
    private YXAudioPlayer player;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video);

        if(Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP){
            if(ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)!= PackageManager.PERMISSION_GRANTED){
                ActivityCompat.requestPermissions(this,PERMISSIONS_STORAGE,REQUEST_PERMISSION_CODE);
            }
        }
        surfaceView = findViewById(R.id.sv);
        player = YXAudioPlayer.getDefault();
        player.setSurfaceView(surfaceView);
        player.setOnPreparedListener(new yxOnPreparedListener() {
            @Override
            public void onPrepared() {
                MyLog.d("onPrepared");
                player.start();

            }
        });
        player.setOnLoadListener(new yxOnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                if(load){
                    MyLog.d("加载中...");
                }else {
                    MyLog.d("播放中...");
                }
            }
        });



        player.setOnPauseResumeListener(new yxOnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if(pause){
                    MyLog.d("暂停中...");
                }else {
                    MyLog.d("播放中...");
                }
            }
        });


        player.setOnErrorListener(new yxOnErrorListener() {
            @Override
            public void onError(int code, String msg) {
                MyLog.d("code: "+code+", msg: "+msg);
            }
        });

        player.setOnCompleteListener(new yxOnCompleteListener() {
            @Override
            public void onComplete() {
                MyLog.d("播放完成");
            }
        });
    }

    public void begin(View view) {
        String path = new File(Environment.getExternalStorageDirectory(),"Android仿微信实现IOS风格的滑动返回.mp4").getAbsolutePath();
        player.setSource(path);
        player.prepared();
    }

    public void pause(View view) {
        player.pause();
    }

    public void resume(View view) {
        player.resume();
    }

    public void stop(View view) {
        player.stop();
    }

    public void next(View view) {
        //player.playNext(AUDIO_SOURCE_TWO);
    }
}
