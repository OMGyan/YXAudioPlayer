//
// Created by Administrator on 2019/11/23.
//

#ifndef YXAUDIOPLAYER_YXAUDIO_H
#define YXAUDIOPLAYER_YXAUDIO_H

#include "YXQueue.h"
#include "YXPlayStatus.h"
#include "YXCallJava.h"


extern "C"{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
};

extern "C" {
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
}


class YXAudio {

public:
    int streamIndex = -1;
    AVCodecParameters *codecpar = NULL;
    AVCodecContext *avCodecContext = NULL;
    YXQueue *yxQueue;
    YXPlayStatus *yxPlayStatus;
    YXCallJava *yxCallJava;
    pthread_t thread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = -1;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0;

    int duration = 0;
    AVRational time_base;
    double now_time = 0;
    double clock = 0;
    double last_time = 0;

    //引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;
    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pclPlayerPlay = NULL;
    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue;

public:
    YXAudio(YXPlayStatus *yxPlayStatus,int sample_rate,YXCallJava *yxcj);
    ~YXAudio();

    void play();
    int reSampleAudio();
    void initOpenSLES();
    int getCurrentSampleRateForOpensles(int sample_rate);
    void pause();
    void resume();
    void stop();
    void release();
};


#endif //YXAUDIOPLAYER_YXAUDIO_H
