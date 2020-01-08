package com.yx.yxaudioplayerlib.listener;

/**
 * Author by YX, Date on 2020/1/8.
 */
public interface yxOnPcmInfoListener {
    void onPcmInfo(byte[] buffer,int bufferSize);
    void onPcmRate(int sampleRate,int bit,int channels);
}
