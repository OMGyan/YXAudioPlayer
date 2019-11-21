//
// Created by Administrator on 2019/11/21.
//

#ifndef YXAUDIOPLAYER_ANDROIDLOG_H
#define YXAUDIOPLAYER_ANDROIDLOG_H

#endif //YXAUDIOPLAYER_ANDROIDLOG_H

#include "android/log.h"

#define LOG_DEBUG true

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"yanxun",FORMAT,##__VA_ARGS__);
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,"yanxun",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"yanxun",FORMAT,##__VA_ARGS__);