#include <jni.h>
#include "YXCallJava.h"
#include "YXFFmpeg.h"


JavaVM *jvm;
YXCallJava *yxCallJava;
YXFFmpeg *yxfFmpeg;
YXPlayStatus *yxPlayStatus;
bool nexit = true;
pthread_t start_thread;

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
        yxCallJava->onCallLoad(MAIN_THREAD, true);
        yxPlayStatus = new YXPlayStatus();
        yxfFmpeg = new YXFFmpeg(yxCallJava,source,yxPlayStatus);
        yxfFmpeg->prepared();
    }

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

void *startThreadCallback(void *data){
    YXFFmpeg *yxfF = static_cast<YXFFmpeg *>(data);
    yxfF->start();
    pthread_exit(&start_thread);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1start(JNIEnv *env, jobject instance) {

    // TODO
    if(yxfFmpeg!=NULL){
        pthread_create(&start_thread,NULL,startThreadCallback,yxfFmpeg);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1pause(JNIEnv *env, jobject instance) {

    // TODO
    if(yxfFmpeg!=NULL){
        yxfFmpeg->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1resume(JNIEnv *env, jobject instance) {

    // TODO
    if(yxfFmpeg!=NULL){
        yxfFmpeg->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1stop(JNIEnv *env, jobject instance) {

    // TODO
    if(!nexit){
        return;
    }

    jclass jcz = env->GetObjectClass(instance);
    jmethodID jm_next = env->GetMethodID(jcz,"onCallNext","()V");

    nexit = false;
    if(yxfFmpeg!=NULL){
        yxfFmpeg->release();
        delete(yxfFmpeg);
        yxfFmpeg = NULL;
        if(yxCallJava!=NULL){
            delete(yxCallJava);
            yxCallJava = NULL;
        }
        if(yxPlayStatus!=NULL){
            delete(yxPlayStatus);
            yxPlayStatus = NULL;
        }
    }
    nexit = true;
    env->CallVoidMethod(instance,jm_next);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1seek(JNIEnv *env, jobject instance,
                                                          jint secds) {

    // TODO
    if(yxfFmpeg!=NULL){
        yxfFmpeg->seek(secds);
    }

}
extern "C"
JNIEXPORT jint JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1duration(JNIEnv *env, jobject instance) {

    // TODO
    if(yxfFmpeg!=NULL){
        return yxfFmpeg->duration;
    }
    return 0;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1volume(JNIEnv *env, jobject instance,
                                                            jint percent) {

    // TODO
    if(yxfFmpeg!=NULL){
        yxfFmpeg->setVolume(percent);
    }
}