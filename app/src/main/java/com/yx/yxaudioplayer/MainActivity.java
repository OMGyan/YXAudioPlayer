package com.yx.yxaudioplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.yx.yxaudioplayerlib.YXTimeInfoBean;
import com.yx.yxaudioplayerlib.listener.yxOnCompleteListener;
import com.yx.yxaudioplayerlib.listener.yxOnErrorListener;
import com.yx.yxaudioplayerlib.listener.yxOnLoadListener;
import com.yx.yxaudioplayerlib.listener.yxOnPauseResumeListener;
import com.yx.yxaudioplayerlib.listener.yxOnPreparedListener;
import com.yx.yxaudioplayerlib.listener.yxOnTimeInfoListener;
import com.yx.yxaudioplayerlib.log.MyLog;
import com.yx.yxaudioplayerlib.player.YXAudioPlayer;
import com.yx.yxaudioplayerlib.util.YXTimeUtil;

public class MainActivity extends AppCompatActivity {

    //读写权限
    private static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE};
    //请求状态码
    private static int REQUEST_PERMISSION_CODE = 1;

    private final String AUDIO_SOURCE = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";
    private final String AUDIO_SOURCE_TWO = "http://ngcdn004.cnr.cn/live/dszs/index.m3u8";

    private YXAudioPlayer player;
    private int position = 0;
    private boolean isSeek;


    private Handler mh = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what){
                case 1:
                    if(!isSeek){
                        YXTimeInfoBean timeInfoBean = (YXTimeInfoBean) msg.obj;
                        tv_time.setText(YXTimeUtil.secdsToDateFormat(timeInfoBean.getTotalTime(), timeInfoBean.getTotalTime())
                                + "/" + YXTimeUtil.secdsToDateFormat(timeInfoBean.getCurrentTime(), timeInfoBean.getTotalTime()));
                        seekBar.setProgress(timeInfoBean.getCurrentTime()*100/timeInfoBean.getTotalTime());
                    }
                    break;
                case 100:
                    tv_volume.setText("音量 : "+msg.arg1);
                    break;
            }

        }
    };
    private TextView tv_time;
    private SeekBar seekBar;
    private TextView tv_volume;
    private SeekBar sb_volume;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tv_time = ((TextView) findViewById(R.id.tv_time));
        seekBar = ((SeekBar) findViewById(R.id.sb));
        tv_volume = ((TextView) findViewById(R.id.tv_volume));
        sb_volume = ((SeekBar) findViewById(R.id.sb_volume));

        if(Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP){
            if(ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)!= PackageManager.PERMISSION_GRANTED){
                ActivityCompat.requestPermissions(this,PERMISSIONS_STORAGE,REQUEST_PERMISSION_CODE);
            }
        }

        player = YXAudioPlayer.getDefault();
       // player.setVolume(50);
      //  tv_volume.setText("音量 : "+player.getVolumePercent());
      //  sb_volume.setProgress(player.getVolumePercent());
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
                Message message = mh.obtainMessage();
                message.obj = yxTimeInfoBean;
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

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if(player.getDuration()>0 && isSeek){
                    position = player.getDuration()*progress / 100;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                  isSeek = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                 player.seek(position);
                 isSeek = false;
            }
        });

        sb_volume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                player.setVolume(progress);
                Message message = mh.obtainMessage();
                message.what = 100;
                message.arg1 = progress;
                mh.sendMessage(message);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

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
        tv_time.setText("00:00/00:00");
        seekBar.setProgress(0);
        player.stop();
    }

    public void next(View view) {
        player.playNext(AUDIO_SOURCE_TWO);
    }

    public void leftChannel(View view) {
    }

    public void stereoChannel(View view) {
    }

    public void rightChannel(View view) {
    }
}
