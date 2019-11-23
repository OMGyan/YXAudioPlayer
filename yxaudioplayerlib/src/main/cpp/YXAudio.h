//
// Created by Administrator on 2019/11/23.
//

#ifndef YXAUDIOPLAYER_YXAUDIO_H
#define YXAUDIOPLAYER_YXAUDIO_H

extern "C"{
#include <libavcodec/avcodec.h>
};


class YXAudio {

public:
    int streamIndex = -1;
    AVCodecParameters *codecpar = NULL;
    AVCodecContext *avCodecContext = NULL;

public:
    YXAudio();
    ~YXAudio();

};


#endif //YXAUDIOPLAYER_YXAUDIO_H
