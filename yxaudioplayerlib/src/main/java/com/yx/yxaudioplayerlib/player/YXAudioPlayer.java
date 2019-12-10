package com.yx.yxaudioplayerlib.player;

import android.text.TextUtils;

import com.yx.yxaudioplayerlib.YXTimeInfoBean;
import com.yx.yxaudioplayerlib.enums.ChannelEnum;
import com.yx.yxaudioplayerlib.listener.yxOnCompleteListener;
import com.yx.yxaudioplayerlib.listener.yxOnErrorListener;
import com.yx.yxaudioplayerlib.listener.yxOnLoadListener;
import com.yx.yxaudioplayerlib.listener.yxOnPauseResumeListener;
import com.yx.yxaudioplayerlib.listener.yxOnPreparedListener;
import com.yx.yxaudioplayerlib.listener.yxOnTimeInfoListener;
import com.yx.yxaudioplayerlib.log.MyLog;

/**
 * Author by YX, Date on 2019/11/21.
 */
public class YXAudioPlayer {

    private yxOnPreparedListener OnPreparedListener;

    private yxOnLoadListener onLoadListener;

    private yxOnPauseResumeListener onPauseResumeListener;

    private yxOnTimeInfoListener onTimeInfoListener;

    private YXTimeInfoBean yxTimeInfoBean;

    private yxOnErrorListener onErrorListener;

    private yxOnCompleteListener onCompleteListener;

    private boolean playNext = false;

    private int duration = -1;

    private int volumePercent = 100;

    //初始化声道
    private ChannelEnum ce = ChannelEnum.CHANNEL_LEFT;

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

    public void setOnLoadListener(yxOnLoadListener onLoadListener) {
        this.onLoadListener = onLoadListener;
    }

    public void setOnPauseResumeListener(yxOnPauseResumeListener onPauseResumeListener) {
        this.onPauseResumeListener = onPauseResumeListener;
    }

    public void setOnTimeInfoListener(yxOnTimeInfoListener onTimeInfoListener) {
        this.onTimeInfoListener = onTimeInfoListener;
    }

    public void setOnErrorListener(yxOnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    public void setOnCompleteListener(yxOnCompleteListener onCompleteListener) {
        this.onCompleteListener = onCompleteListener;
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

    public void onCallLoad(boolean load){
        if(onLoadListener!=null){
            onLoadListener.onLoad(load);
        }
    }

    public void onCallError(int code,String msg){
        stop();
        if(onErrorListener!=null){
            onErrorListener.onError(code,msg);
        }
    }

    public void onCallComplete(){
        stop();
        if(onCompleteListener!=null){
            onCompleteListener.onComplete();
        }
    }

    public void onCallTimeInfo(int currentTime,int totalTime){
        if(onTimeInfoListener!=null){
            if(yxTimeInfoBean == null){
                yxTimeInfoBean = new YXTimeInfoBean();
            }
            yxTimeInfoBean.setCurrentTime(currentTime);
            yxTimeInfoBean.setTotalTime(totalTime);
            onTimeInfoListener.onTimeInfo(yxTimeInfoBean);
        }
    }

    public void onCallNext(){
        //如果当前是进入下一曲
        if(playNext){
            playNext = !playNext;
            prepared();
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

    public void start(){
        if(TextUtils.isEmpty(source)){
            MyLog.d("source can not be empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                setVolume(volumePercent);
                setMute(ce);
                n_start();
            }
        }).start();
    }

    public void pause(){
        n_pause();
        if(onPauseResumeListener!=null){
            onPauseResumeListener.onPause(true);
        }
    }

    public void resume(){
        n_resume();
        if(onPauseResumeListener!=null){
            onPauseResumeListener.onPause(false);
        }
    }

    public void stop(){
        yxTimeInfoBean = null;
        duration = -1;
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
            }
        }).start();
    }

    public void playNext(String url){
        source = url;
        playNext = true;
        stop();
    }

    public int getDuration(){
        if(duration<0){
            duration = n_duration();
        }
        return duration;
    }

    public void setVolume(int percent){
        if(percent>=0 && percent<=100){
            volumePercent = percent;
            n_volume(percent);
        }
    }

    public void setMute(ChannelEnum channelEnum){
        ce = channelEnum;
        n_mute(channelEnum.getValue());
    }

    public int getVolumePercent() {
        return volumePercent;
    }

    public void seek(int secds){
        n_seek(secds);
    }

    private native void n_prepared(String source);

    private native void n_start();

    private native void n_pause();

    private native void n_resume();

    private native void n_stop();

    private native void n_seek(int secds);

    private native int n_duration();

    private native void n_volume(int percent);

    private native void n_mute(int mute);
}
