//
// Created by Administrator on 2019/11/22.
//

#ifndef YXAUDIOPLAYER_YXCALLJAVA_H
#define YXAUDIOPLAYER_YXCALLJAVA_H
#define MAIN_THREAD 0
#define CHILD_THREAD 1

#define CAN_NOT_OPEN_URL 1001
#define CAN_NOT_FIND_STREAM_FROM_URL 1002
#define CAN_NOT_FIND_DECODER 1003
#define CAN_NOT_ALLOC_DECODER_CONTEXT 1004
#define CAN_NOT_FILL_DECODER_CONTEXT 1005
#define CAN_NOT_OPEN_AUDIO_STREAMS 1006
#define YXAUDIO_IS_NULL 1007

#define RIGHT_CHANNEL 0
#define LEFT_CHANNEL 1
#define STEREO_CHANNEL 2


#include "jni.h"

class YXCallJava {

public:

    JavaVM *jvm;
    JNIEnv *jev;
    jobject jobj;
    jmethodID jm_prepared;
    jmethodID jm_load;
    jmethodID jm_timeinfo;
    jmethodID jm_error;
    jmethodID jm_complete;
    jmethodID jm_valuedb;
    jmethodID jm_pcmtoaac;
    jmethodID jm_pcminfo;
    jmethodID jm_pcmsamplerate;
    jmethodID jm_rendereryuv;

public:

    YXCallJava(JavaVM *vm,JNIEnv *jniEnv,jobject job);
    ~YXCallJava();
    void onCallPrepared(int threadType);
    void onCallLoad(int threadType, bool load);
    void onCallTimeInfo(int threadType,int curr,int total);
    void onCallError(int threadType,int code,const char *msg);
    void onCallComplete(int threadType);
    void onCallValueDB(int threadType,int db);
    void onCallPcmToAAC(int threadType,int size, void *buffer);
    void onCallPcmInfo(void *buffer,int size);
    void onCallPcmRate(int samplerate);
    void onCallRendererYUV(int width,int height,uint8_t *fy,uint8_t *fu,uint8_t *fv);

};


#endif //YXAUDIOPLAYER_YXCALLJAVA_H
