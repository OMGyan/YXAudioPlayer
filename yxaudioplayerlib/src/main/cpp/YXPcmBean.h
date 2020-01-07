//
// Created by Administrator on 2019/12/30.
//

#ifndef YXAUDIOPLAYER_YXPCMBEAN_H
#define YXAUDIOPLAYER_YXPCMBEAN_H

#include <SoundTouch.h>
using namespace soundtouch;

class YXPcmBean {

public:
    //声明一个缓冲
    char *buffer;
    //缓冲大小
    int buffsize;

public:
    //构造方法
    YXPcmBean(SAMPLETYPE *buffer,int size);
    //虚构函数
    ~YXPcmBean();
};


#endif //YXAUDIOPLAYER_YXPCMBEAN_H
