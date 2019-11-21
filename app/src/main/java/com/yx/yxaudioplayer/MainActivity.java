package com.yx.yxaudioplayer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;


import com.yx.yxaudioplayerlib.Demo;
import com.yx.yxaudioplayerlib.player.YXAudioPlayer;

public class MainActivity extends AppCompatActivity {

    private final String AUDIO_SOURCE = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";

    private YXAudioPlayer player;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        player = YXAudioPlayer.getDefault();

    }


    public void begin(View view) {
       player.setSource(AUDIO_SOURCE);
       player.prepared();
    }
}
