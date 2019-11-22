//
// Created by Administrator on 2019/11/22.
//

#ifndef YXAUDIOPLAYER_YXFFMPEG_H
#define YXAUDIOPLAYER_YXFFMPEG_H

#include "YXCallJava.h"
#include "pthread.h"
#include "AndroidLog.h"

extern "C"{
#include <libavformat/avformat.h>
};


class YXFFmpeg {

public:
    YXCallJava *yxCallJava = NULL;
    const char *url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFormatCtx = NULL;
public:
    YXFFmpeg(YXCallJava *ycjava, const char *url);
    ~YXFFmpeg();
    void prepared();
    void decodeFFmpegThread();
};


#endif //YXAUDIOPLAYER_YXFFMPEG_H
