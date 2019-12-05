//
// Created by Administrator on 2019/11/27.
//

#ifndef YXAUDIOPLAYER_YXPLAYSTATUS_H
#define YXAUDIOPLAYER_YXPLAYSTATUS_H


class YXPlayStatus {

public:
    bool exit;
    bool load = true;
    bool seek = false;
public:
    YXPlayStatus();
    ~YXPlayStatus();
};


#endif //YXAUDIOPLAYER_YXPLAYSTATUS_H
