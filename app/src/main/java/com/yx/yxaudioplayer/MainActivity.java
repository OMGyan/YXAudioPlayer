package com.yx.yxaudioplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v7.widget.AppCompatSeekBar;
import android.view.View;

import com.yx.yxaudioplayerlib.YXTimeInfoBean;
import com.yx.yxaudioplayerlib.listener.yxOnCompleteListener;
import com.yx.yxaudioplayerlib.listener.yxOnErrorListener;
import com.yx.yxaudioplayerlib.listener.yxOnLoadListener;
import com.yx.yxaudioplayerlib.listener.yxOnPauseResumeListener;
import com.yx.yxaudioplayerlib.listener.yxOnPreparedListener;
import com.yx.yxaudioplayerlib.listener.yxOnTimeInfoListener;
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
   // private final String AUDIO_SOURCE = "http://ngcdn004.cnr.cn/live/dszs/index.m3u8";

    private YXAudioPlayer player;
    private AppCompatSeekBar seekBar;
    private Handler mh = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            seekBar.setProgress(msg.arg1);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        seekBar = ((AppCompatSeekBar) findViewById(R.id.sb));


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

        player.setOnTimeInfoListener(new yxOnTimeInfoListener() {
            @Override
            public void onTimeInfo(YXTimeInfoBean yxTimeInfoBean) {
                MyLog.d(yxTimeInfoBean.toString());
                if(seekBar.getProgress()==0){
                    seekBar.setMax(yxTimeInfoBean.getTotalTime());
                }
                Message message = mh.obtainMessage();
                message.arg1 = yxTimeInfoBean.getCurrentTime();
                message.what = 1;
                mh.sendMessage(message);
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
       player.setSource(AUDIO_SOURCE);
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

    public void seek(View view) {
        player.seek(215);
    }
}
