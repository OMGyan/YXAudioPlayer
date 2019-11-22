//
// Created by Administrator on 2019/11/22.
//

#ifndef YXAUDIOPLAYER_YXCALLJAVA_H
#define YXAUDIOPLAYER_YXCALLJAVA_H
#define MAIN_THREAD 0
#define CHILD_THREAD 1

#include "jni.h"

class YXCallJava {

public:
    JavaVM *jvm;
    JNIEnv *jev;
    jobject jobj;
    jmethodID jm_prepared;

public:
    YXCallJava(JavaVM *vm,JNIEnv *jniEnv,jobject job);
    ~YXCallJava();
     void onCallPrepared(int threadType);
};


#endif //YXAUDIOPLAYER_YXCALLJAVA_H
