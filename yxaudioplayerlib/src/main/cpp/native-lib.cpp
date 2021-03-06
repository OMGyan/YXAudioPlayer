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

    if(yxfFmpeg!=NULL){
        pthread_create(&start_thread,NULL,startThreadCallback,yxfFmpeg);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1pause(JNIEnv *env, jobject instance) {

    if(yxfFmpeg!=NULL){
        yxfFmpeg->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1resume(JNIEnv *env, jobject instance) {

    if(yxfFmpeg!=NULL){
        yxfFmpeg->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1stop(JNIEnv *env, jobject instance) {

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
    if(yxfFmpeg!=NULL){
        yxfFmpeg->seek(secds);
    }

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1duration(JNIEnv *env, jobject instance) {

    if(yxfFmpeg!=NULL){
        return yxfFmpeg->duration;
    }
    return 0;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1volume(JNIEnv *env, jobject instance,
                                                            jint percent) {
    if(yxfFmpeg!=NULL){
        yxfFmpeg->setVolume(percent);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1mute(JNIEnv *env, jobject instance,
                                                          jint mute) {
    if(yxfFmpeg!=NULL){
        yxfFmpeg->setMute(mute);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1pitch(JNIEnv *env, jobject instance,
                                                           jfloat pitch) {
    if(yxfFmpeg!=NULL){
        yxfFmpeg->setPitch(pitch);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1speed(JNIEnv *env, jobject instance,
                                                           jfloat speed) {
    if(yxfFmpeg!=NULL){
        yxfFmpeg->setSpeed(speed);
    }

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1samplerate(JNIEnv *env, jobject instance) {

   if(yxfFmpeg!=NULL){
       return yxfFmpeg->getSampleRate();
   }
    return 0;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1startStopRecord(JNIEnv *env, jobject instance,
                                                                     jboolean start) {
     if(yxfFmpeg!=NULL){
         yxfFmpeg->startStopRecord(start);
     }

}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1cutaudio(JNIEnv *env, jobject instance,
                                                              jint start_time, jint end_time,
                                                              jboolean showPcm) {
    // TODO
    if(yxfFmpeg != NULL){
        return yxfFmpeg->cutAudio(start_time,end_time,showPcm);

    }
    return false;
}