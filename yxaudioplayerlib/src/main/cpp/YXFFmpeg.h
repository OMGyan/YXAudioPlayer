//
// Created by Administrator on 2019/11/22.
//

#ifndef YXAUDIOPLAYER_YXFFMPEG_H
#define YXAUDIOPLAYER_YXFFMPEG_H

#include "YXCallJava.h"
#include "pthread.h"
#include "AndroidLog.h"
#include "YXAudio.h"
#include "YXVideo.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
};


class YXFFmpeg {

public:
    YXCallJava *yxCallJava = NULL;
    const char *url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFormatCtx = NULL;
    YXAudio *yxAudio = NULL;
    //声明video类
    YXVideo *video = NULL;
    YXPlayStatus *status;
    pthread_mutex_t init_mutex;
    bool exit = false;
    int duration = 0;
    pthread_mutex_t seek_mutex;

public:
    YXFFmpeg(YXCallJava *ycjava, const char *url,YXPlayStatus *status);
    ~YXFFmpeg();
    void prepared();
    void decodeFFmpegThread();
    void start();
    void pause();
    void resume();
    void release();
    void seek(int64_t secds);
    void setVolume(int percent);
    void setMute(int mute);
    void setPitch(float pitch);

    void setSpeed(float speed);

    int getSampleRate();

    void startStopRecord(bool start);

    bool cutAudio(int start_time,int end_time, bool isShowPcm);

    int getCodecContext(AVCodecParameters *codecParameters,AVCodecContext **codecContext);
};


#endif //YXAUDIOPLAYER_YXFFMPEG_H
