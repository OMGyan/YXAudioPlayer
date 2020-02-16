//
// Created by Administrator on 2019/12/30.
//

#ifndef YXAUDIOPLAYER_YXBUFFERQUEUE_H
#define YXAUDIOPLAYER_YXBUFFERQUEUE_H


#include <deque>
#include "YXPcmBean.h"
#include "AndroidLog.h"
#include "YXPlayStatus.h"
#include <pthread.h>

class YXBufferQueue {
public:
    std::deque<YXPcmBean *> queueBuffer;
    pthread_mutex_t mutexBuffer;
    pthread_cond_t condBuffer;
    YXPlayStatus *yxPlayStatus = NULL;
public:
    YXBufferQueue(YXPlayStatus *yxPlayStatus);
    ~YXBufferQueue();
    int putBuffer(SAMPLETYPE *buffer,int size);
    int getBuffer(YXPcmBean **yxPcmBean);
    int clearBuffer();
    void release();
    int getBufferSize();
    int noticeThread();
};


#endif //YXAUDIOPLAYER_YXBUFFERQUEUE_H
