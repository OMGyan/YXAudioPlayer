//
// Created by Administrator on 2020/1/8.
//

#include "YXQueue.h"
#include "YXCallJava.h"

#ifndef YXAUDIOPLAYER_YXVIDEO_H

//由于FFmpeg是C语言编写的,又因为这里是C++环境,所以下面这样做
extern "C"{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
};
#define YXAUDIOPLAYER_YXVIDEO_H

class YXVideo {

public:
    //视频流索引
    int videoStreamIndex = -1;
    //声明解码器上下文
    AVCodecContext *avCodecContext = NULL;
    //声明解码器属性
    AVCodecParameters *avCodecParameters = NULL;
    //声明存储AVpacket的队列
    YXQueue *queue = NULL;
    //声明播放状态
    YXPlayStatus *playStatus = NULL;
    //初始化调用Java方法类
    YXCallJava *callJava = NULL;
    //声明时间刻度
    AVRational time_base;
    //创建线程用来获取队列里的数据
    pthread_t thread_play;

public:
    //创建构造方法
    YXVideo(YXPlayStatus *yxPlayStatus,YXCallJava *yxCallJava);
    //创建虚构方法(用来释放资源)
    ~YXVideo();
    //播放方法
    void play();
    //释放资源内存
    void release();
};


#endif //YXAUDIOPLAYER_YXVIDEO_H
