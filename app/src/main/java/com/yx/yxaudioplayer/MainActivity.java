package com.yx.yxaudioplayer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;



import com.yx.yxaudioplayerlib.Demo;

public class MainActivity extends AppCompatActivity {


    private Demo demo;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        demo = new Demo();

        demo.testFfmpeg();

    }


}
