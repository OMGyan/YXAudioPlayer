#include <jni.h>
#include "YXCallJava.h"
#include "YXFFmpeg.h"


JavaVM *jvm;
YXCallJava *yxCallJava;
YXFFmpeg *yxfFmpeg;


extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1prepared(JNIEnv *env, jobject instance,
                                                              jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);
    // TODO
    if(yxfFmpeg==NULL){
        if(yxCallJava==NULL){
            yxCallJava = new YXCallJava(jvm,env,instance);
        }
        yxfFmpeg = new YXFFmpeg(yxCallJava,source);
        yxfFmpeg->prepared();
    }

    //yxCallJava->onCallPrepared(MAIN_THREAD);

    env->ReleaseStringUTFChars(source_, source);
}





JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *VM, void *reserved){
    jint result = -1;
    jvm = VM;
    JNIEnv *jev;
    if(VM->GetEnv(reinterpret_cast<void **>(jev), JNI_VERSION_1_4) != JNI_OK){
        return result;
    }
    return JNI_VERSION_1_4;
}