//
// Created by Administrator on 2019/11/27.
//

#ifndef YXAUDIOPLAYER_YXQUEUE_H
#define YXAUDIOPLAYER_YXQUEUE_H

#include "queue"
#include "pthread.h"
#include "AndroidLog.h"
#include "YXPlayStatus.h"

extern "C"{
#include "libavcodec/avcodec.h"
};

class YXQueue {

public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    YXPlayStatus *yxPlayStatus;

public:
    YXQueue(YXPlayStatus *yxPlayStatus);
    ~YXQueue();

    int putAvpacket(AVPacket *packet);
    int getAvpacket(AVPacket *packet);
    int getQueueSize();

};


#endif //YXAUDIOPLAYER_YXQUEUE_H
