//
// Created by Administrator on 2019/11/27.
//

#include "YXQueue.h"

YXQueue::YXQueue(YXPlayStatus *yxPlayStatus) {
    this->yxPlayStatus = yxPlayStatus;
    pthread_mutex_init(&mutexPacket,NULL);
    pthread_cond_init(&condPacket,NULL);
}

YXQueue::~YXQueue() {
    clearAvpacket();
    pthread_mutex_destroy(&mutexPacket);
    pthread_cond_destroy(&condPacket);
}

int YXQueue::putAvpacket(AVPacket *packet) {
    //加锁
    pthread_mutex_lock(&mutexPacket);
    //入队
    queuePacket.push(packet);
    //发送消息给消费者
    pthread_cond_signal(&condPacket);
    //解锁
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int YXQueue::getAvpacket(AVPacket *packet) {
    //加锁
    pthread_mutex_lock(&mutexPacket);
    while(yxPlayStatus !=NULL && !yxPlayStatus->exit){
        if(queuePacket.size()>0){
            //拿出队头
            AVPacket *avPacket = queuePacket.front();
            if(av_packet_ref(packet,avPacket)==0){
                //出队
                queuePacket.pop();
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            break;
        } else{
            //等待生产者生产
            pthread_cond_wait(&condPacket,&mutexPacket);
        }
    }
    //解锁
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int YXQueue::getQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}

void YXQueue::clearAvpacket() {
    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);
    while (!queuePacket.empty()){
        AVPacket *packet = queuePacket.front();
        queuePacket.pop();
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    pthread_mutex_unlock(&mutexPacket);
}


