package com.yx.yxaudioplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import com.yx.yxaudioplayerlib.listener.yxOnPreparedListener;
import com.yx.yxaudioplayerlib.log.MyLog;
import com.yx.yxaudioplayerlib.player.YXAudioPlayer;

public class MainActivity extends AppCompatActivity {

    //读写权限
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE};
    //请求状态码
    private static int REQUEST_PERMISSION_CODE = 1;

    private final String AUDIO_SOURCE = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";

    private YXAudioPlayer player;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        if(Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP){
            if(ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)!= PackageManager.PERMISSION_GRANTED){
                ActivityCompat.requestPermissions(this,PERMISSIONS_STORAGE,REQUEST_PERMISSION_CODE);
            }
        }

        player = YXAudioPlayer.getDefault();
        player.setOnPreparedListener(new yxOnPreparedListener() {
            @Override
            public void onPrepared() {
                MyLog.d("onPrepared");
                player.start();
            }
        });


    }


    public void begin(View view) {
       player.setSource(AUDIO_SOURCE);
       player.prepared();
    }
}
