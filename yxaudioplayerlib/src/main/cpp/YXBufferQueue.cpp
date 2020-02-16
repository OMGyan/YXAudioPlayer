//
// Created by Administrator on 2019/12/30.
//


#include "YXBufferQueue.h"

YXBufferQueue::YXBufferQueue(YXPlayStatus *yxPlayStatus) {
    this->yxPlayStatus = yxPlayStatus;
    pthread_mutex_init(&mutexBuffer, NULL);
    pthread_cond_init(&condBuffer, NULL);
}

YXBufferQueue::~YXBufferQueue() {
    yxPlayStatus = NULL;
    pthread_mutex_destroy(&mutexBuffer);
    pthread_cond_destroy(&condBuffer);
    if(LOG_DEBUG)
    {
        LOGE("YXBufferQueue 释放完了");
    }
}

int YXBufferQueue::putBuffer(SAMPLETYPE *buffer, int size) {
    pthread_mutex_lock(&mutexBuffer);
    YXPcmBean *yxPcmBean = new YXPcmBean(buffer,size);
    queueBuffer.push_back(yxPcmBean);
    pthread_cond_signal(&condBuffer);
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int YXBufferQueue::getBuffer(YXPcmBean **yxPcmBean) {
    pthread_mutex_lock(&mutexBuffer);
    while(yxPlayStatus!=NULL && !yxPlayStatus->exit){
        if(queueBuffer.size()>0){
           *yxPcmBean = queueBuffer.front();
           queueBuffer.pop_front();
           break;
        } else{
            if(!yxPlayStatus->exit)
            {
                pthread_cond_wait(&condBuffer, &mutexBuffer);
            }
        }
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int YXBufferQueue::clearBuffer() {
    pthread_cond_signal(&condBuffer);
    pthread_mutex_lock(&mutexBuffer);
    while (!queueBuffer.empty())
    {
        YXPcmBean *pcmBean = queueBuffer.front();
        queueBuffer.pop_front();
        delete(pcmBean);
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int YXBufferQueue::getBufferSize() {
    int size = 0;
    pthread_mutex_lock(&mutexBuffer);
    size = queueBuffer.size();
    pthread_mutex_unlock(&mutexBuffer);
    return size;

}

int YXBufferQueue::noticeThread() {
    pthread_cond_signal(&condBuffer);
    return 0;
}

void YXBufferQueue::release() {
    if(LOG_DEBUG){
        LOGE("YXBufferQueue::release");
    }
    noticeThread();
    clearBuffer();
    if(LOG_DEBUG){
        LOGE("YXBufferQueue::release success");
    }
}
