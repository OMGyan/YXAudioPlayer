package com.yx.yxaudioplayer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;

import com.yx.yxaudioplayerlib.YXTimeInfoBean;
import com.yx.yxaudioplayerlib.listener.yxOnPcmInfoListener;
import com.yx.yxaudioplayerlib.listener.yxOnPreparedListener;
import com.yx.yxaudioplayerlib.listener.yxOnTimeInfoListener;
import com.yx.yxaudioplayerlib.log.MyLog;
import com.yx.yxaudioplayerlib.player.YXAudioPlayer;

public class CutAudioActivity extends AppCompatActivity {
    private YXAudioPlayer player;
    private final String AUDIO_SOURCE = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cut_audio);
        player = YXAudioPlayer.getDefault();

        player.setOnPreparedListener(new yxOnPreparedListener() {
            @Override
            public void onPrepared() {
                player.cutAudioPlay(10,40,true);
            }
        });

        player.setOnTimeInfoListener(new yxOnTimeInfoListener() {
            @Override
            public void onTimeInfo(YXTimeInfoBean yxTimeInfoBean) {
                MyLog.d(yxTimeInfoBean.toString());
            }
        });

        player.setOnPcmInfoListener(new yxOnPcmInfoListener() {
            @Override
            public void onPcmInfo(byte[] buffer, int bufferSize) {
                MyLog.d("bufferSize : "+bufferSize);
            }

            @Override
            public void onPcmRate(int sampleRate, int bit, int channels) {
                MyLog.d("sampleRate : "+sampleRate);
            }

        });
    }


    public void cutAudio(View view) {
        player.setSource(AUDIO_SOURCE);
        player.prepared();
    }
}
