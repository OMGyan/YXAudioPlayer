//
// Created by Administrator on 2019/11/23.
//

#ifndef YXAUDIOPLAYER_YXAUDIO_H
#define YXAUDIOPLAYER_YXAUDIO_H

#include "YXQueue.h"
#include "YXPlayStatus.h"
#include "YXCallJava.h"
#include "SoundTouch.h"
#include "YXBufferQueue.h"
#include "YXPcmBean.h"

using namespace soundtouch;


extern "C"{
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/time.h>
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
    int volumePercent = 100;
    int mute = 2;

    float pitch = 1.0f;
    float speed = 1.0f;

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
    SLVolumeItf pcmVolumePlay = NULL;
    SLMuteSoloItf pcmMutePlay = NULL;
    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    //1.声明SoundTouch对象和内存变量：
    SoundTouch *soundTouch = NULL;
    SAMPLETYPE *sampleBuffer = NULL;
    //处理PCM数据是否完成
    bool finished = true;
    uint8_t *out_buffer = NULL;
    int nb = 0;
    int num = 0;
    bool isRecord = false;
    bool readFrameFinished = true;
    bool isCut = false;
    int end_time = 0;
    bool showPcm = false;

    pthread_t pcmCallBackThread;
    YXBufferQueue *bufferQueue;
    int defaultPcmSize = 4096;

public:
    YXAudio(YXPlayStatus *yxPlayStatus,int sample_rate,YXCallJava *yxcj);
    ~YXAudio();

    void play();
    int reSampleAudio(void **pcmbuf);
    void initOpenSLES();
    int getCurrentSampleRateForOpensles(int sample_rate);
    void pause();
    void resume();
    void stop();
    void release();
    void setVolume(int percent);
    void setMute(int mute);
    int getSoundTouchData();
    void setPitch(float pitch);
    void setSpeed(float speed);
    //计算pcm数据的分贝值(声音的振幅)
    int getPCMDB(char *pcmdata,size_t pcmsize);

    void startStopRecord(bool start);
};


#endif //YXAUDIOPLAYER_YXAUDIO_H
