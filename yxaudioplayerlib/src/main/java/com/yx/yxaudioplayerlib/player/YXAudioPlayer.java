package com.yx.yxaudioplayerlib.player;

import android.text.TextUtils;

import com.yx.yxaudioplayerlib.listener.yxOnPreparedListener;
import com.yx.yxaudioplayerlib.log.MyLog;

/**
 * Author by YX, Date on 2019/11/21.
 */
public class YXAudioPlayer {

    private yxOnPreparedListener OnPreparedListener;

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("swscale-4");
        System.loadLibrary("postproc-54");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avdevice-57");
    }



    private YXAudioPlayer(){}

    private static class SingleClass{
        private static final YXAudioPlayer yxap = new YXAudioPlayer();
    }

    public static YXAudioPlayer getDefault(){
        return SingleClass.yxap;
    }

    public void setOnPreparedListener(yxOnPreparedListener onPreparedListener) {
        OnPreparedListener = onPreparedListener;
    }

    private String source;

    public void setSource(String source) {
        this.source = source;
    }

    public void onCallPrepared(){
        if(OnPreparedListener!=null){
            OnPreparedListener.onPrepared();
        }
    }

    public void prepared(){
       if(TextUtils.isEmpty(source)){
           MyLog.d("source can not be empty");
           return;
       }

       new Thread(new Runnable() {
           @Override
           public void run() {
              n_prepared(source);
           }
       }).start();

    }

    public native void n_prepared(String source);
}
