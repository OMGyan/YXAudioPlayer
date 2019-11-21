#include <jni.h>

extern "C"
JNIEXPORT void JNICALL
Java_com_yx_yxaudioplayerlib_player_YXAudioPlayer_n_1prepared(JNIEnv *env, jobject instance,
                                                              jstring source_) {
    const char *source = env->GetStringUTFChars(source_, 0);

    // TODO

    env->ReleaseStringUTFChars(source_, source);
}