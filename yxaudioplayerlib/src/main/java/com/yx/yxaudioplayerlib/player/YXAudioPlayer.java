package com.yx.yxaudioplayerlib.player;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.text.TextUtils;

import com.yx.yxaudioplayerlib.YXTimeInfoBean;
import com.yx.yxaudioplayerlib.enums.ChannelEnum;
import com.yx.yxaudioplayerlib.listener.yxOnCompleteListener;
import com.yx.yxaudioplayerlib.listener.yxOnErrorListener;
import com.yx.yxaudioplayerlib.listener.yxOnLoadListener;
import com.yx.yxaudioplayerlib.listener.yxOnPauseResumeListener;
import com.yx.yxaudioplayerlib.listener.yxOnPcmInfoListener;
import com.yx.yxaudioplayerlib.listener.yxOnPreparedListener;
import com.yx.yxaudioplayerlib.listener.yxOnRecordTimeListener;
import com.yx.yxaudioplayerlib.listener.yxOnTimeInfoListener;
import com.yx.yxaudioplayerlib.listener.yxOnValueDBListener;
import com.yx.yxaudioplayerlib.log.MyLog;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

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

    private yxOnValueDBListener onValueDBListener;

    private yxOnRecordTimeListener onRecordTimeListener;

    private yxOnPcmInfoListener onPcmInfoListener;

    private boolean playNext = false;

    private int duration = -1;

    private int volumePercent = 100;

    private float speed = 1.0f;
    private float pitch = 1.0f;
    private boolean initMediaCodec = false;

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

    public void setOnValueDBListener(yxOnValueDBListener onValueDBListener) {
        this.onValueDBListener = onValueDBListener;
    }

    public void setOnRecordTimeListener(yxOnRecordTimeListener onRecordTimeListener) {
        this.onRecordTimeListener = onRecordTimeListener;
    }

    public void setOnPcmInfoListener(yxOnPcmInfoListener onPcmInfoListener) {
        this.onPcmInfoListener = onPcmInfoListener;
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

    public void onCallValueDB(int db){
        if(onValueDBListener!=null){
            onValueDBListener.onDBValue(db);
        }
    }

    public void onCallPcmInfo(byte[] buffer,int bufferSize){
        if(onPcmInfoListener!=null){
           onPcmInfoListener.onPcmInfo(buffer,bufferSize);
        }
    }

    public void onCallPcmRate(int sampleRate){
        if(onPcmInfoListener!=null){
            onPcmInfoListener.onPcmRate(sampleRate,16,2);
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
                setPitch(pitch);
                setSpeed(speed);
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
        stopRecord();
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

    public void setPitch(float p)
    {
        pitch = p;
        n_pitch(pitch);
    }

    public void setSpeed(float s)
    {
        speed = s;
        n_speed(speed);
    }

    public void startRecord(File outFile){
        if(!initMediaCodec){
            audioSampleRate = n_samplerate();
            if(audioSampleRate > 0){
                initMediaCodec = !initMediaCodec;
                initMediaCodec(audioSampleRate,outFile);
                n_startStopRecord(true);
                MyLog.d("开始录制");
            }
        }
    }

    public void stopRecord(){
        if(initMediaCodec){
            n_startStopRecord(false);
            releaseMeidaCodec();
        }
    }

    public void pauseRecord(){
        n_startStopRecord(false);
        MyLog.d("暂停录制");
    }

    public void resumeRecord(){
        n_startStopRecord(true);
        MyLog.d("继续录制");
    }

    public void cutAudioPlay(int start_time,int end_time,boolean showPcm){
        if(n_cutaudio(start_time,end_time,showPcm)){
            start();
        }else {
            stop();
            onCallError(100,"cuteaudio params is wrong");
        }
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

    private native void n_pitch(float pitch);

    private native void n_speed(float speed);

    private native int n_samplerate();

    private native void n_startStopRecord(boolean start);

    private native boolean n_cutaudio(int start_time,int end_time,boolean showPcm);

    //MediaCodec

    private MediaFormat encoderFormat = null;
    private MediaCodec encoder = null;
    private FileOutputStream outputStream = null;
    private MediaCodec.BufferInfo info = null;
    private int perPcmSize = 0;
    private byte[] outByteBuffer = null;
    private int aacSampleRate = 4;
    private double recordTime = 0;
    private int audioSampleRate = 0;

    private void initMediaCodec(int sampleRate, File outFile){
        try {
            aacSampleRate = getADTSsamplerate(sampleRate);
            encoderFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC,sampleRate,2);
            encoderFormat.setInteger(MediaFormat.KEY_BIT_RATE,96000);
            encoderFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            encoderFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE,4096);
            encoder = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            info = new MediaCodec.BufferInfo();
            if(encoder == null){
                MyLog.d("create encoder fail");
                return;
            }
            recordTime = 0;
            encoder.configure(encoderFormat,null,null,MediaCodec.CONFIGURE_FLAG_ENCODE);
            outputStream = new FileOutputStream(outFile);
            encoder.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void encodePcmToAAC(int size,byte[] buffer){
        if(buffer!=null && encoder!=null){
            recordTime += size * 1.0/(audioSampleRate * 2 * (16 / 8));
            MyLog.d("recordTime : "+recordTime);
            if(onRecordTimeListener!=null){
                onRecordTimeListener.onRecordTime((int) recordTime);
            }

            int inputBufferIndex = encoder.dequeueInputBuffer(0);
            if(inputBufferIndex >= 0){
                ByteBuffer byteBuffer = encoder.getInputBuffers()[inputBufferIndex];
                byteBuffer.clear();
                byteBuffer.put(buffer);
                encoder.queueInputBuffer(inputBufferIndex,0,size,0,0);
            }
            int index = encoder.dequeueOutputBuffer(info, 0);
            while (index >= 0 && initMediaCodec){
                try {
                    perPcmSize = info.size + 7;
                    outByteBuffer = new byte[perPcmSize];
                    ByteBuffer byteBuffer = encoder.getOutputBuffers()[index];
                    byteBuffer.position(info.offset);
                    byteBuffer.limit(info.offset + info.size);
                    addADTSHeader(outByteBuffer,perPcmSize,aacSampleRate);
                    byteBuffer.get(outByteBuffer,7,info.size);
                    byteBuffer.position(info.offset);
                    outputStream.write(outByteBuffer,0,perPcmSize);
                    encoder.releaseOutputBuffer(index,0);
                    index = encoder.dequeueOutputBuffer(info,0);
                    outByteBuffer = null;
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void addADTSHeader(byte[] packet,int packetLen,int sampleRate){
        int profile = 2; // AAC LC
        int freqIdx = sampleRate; // samplerate
        int chanCfg = 2; // CPE

        packet[0] = (byte) 0xFF; // 0xFFF(12bit) 这里只取了8位，所以还差4位放到下一个里面
        packet[1] = (byte) 0xF9; // 第一个t位放F
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }

    private int getADTSsamplerate(int samplerate)
    {
        int rate = 4;
        switch (samplerate)
        {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }

    private void releaseMeidaCodec(){
        if(encoder == null)
        {
            return;
        }
        try {
            recordTime = 0;
            outputStream.close();
            outputStream = null;
            encoder.stop();
            encoder.release();
            encoder = null;
            encoderFormat = null;
            info = null;
            initMediaCodec = false;

            MyLog.d("录制完成...");
        } catch (IOException e) {
            e.printStackTrace();
        }
        finally {
            if(outputStream != null)
            {
                try {
                    outputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
                outputStream = null;
            }
        }

    }
}
