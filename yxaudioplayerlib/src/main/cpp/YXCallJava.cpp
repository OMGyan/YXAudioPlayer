//
// Created by Administrator on 2019/11/22.
//

#include "YXCallJava.h"
#include "AndroidLog.h"

YXCallJava::YXCallJava(JavaVM *vm,JNIEnv *jniEnv, jobject job) {
   jvm = vm;
   jev = jniEnv;
   jobj = jev->NewGlobalRef(job);

   jclass jcz = jev->GetObjectClass(jobj);

    if(!jcz){
        if(LOG_DEBUG){
            LOGE("get jclass wrong")
        }
        return ;
    }

    jm_prepared = jev->GetMethodID(jcz,"onCallPrepared","()V");
    jm_load = jev->GetMethodID(jcz,"onCallLoad","(Z)V");
    jm_timeinfo = jev->GetMethodID(jcz,"onCallTimeInfo","(II)V");
    jm_error = jev->GetMethodID(jcz,"onCallError","(ILjava/lang/String;)V");
    jm_complete = jev->GetMethodID(jcz,"onCallComplete","()V");
    jm_valuedb = jev->GetMethodID(jcz,"onCallValueDB","(I)V");
    jm_pcmtoaac = jev->GetMethodID(jcz,"encodePcmToAAC","(I[B)V");
    jm_pcminfo = jev->GetMethodID(jcz,"onCallPcmInfo","([BI)V");
    jm_pcmsamplerate = jev->GetMethodID(jcz,"onCallPcmRate","(I)V");
}

YXCallJava::~YXCallJava() {

}

void YXCallJava::onCallPrepared(int threadType) {
   switch (threadType){
       case MAIN_THREAD:
       jev->CallVoidMethod(jobj,jm_prepared);
           break;
       case CHILD_THREAD:
       JNIEnv *ev;
       if(jvm->AttachCurrentThread(&ev,0)!=JNI_OK){
           if(LOG_DEBUG){
               LOGE("get child thread jnienv wrong");
           }
           return;
       }
       ev->CallVoidMethod(jobj,jm_prepared);
       jvm->DetachCurrentThread();
           break;
   }
}

void YXCallJava::onCallLoad(int threadType, bool load) {
    switch (threadType){
        case MAIN_THREAD:
            jev->CallVoidMethod(jobj,jm_load,load);
            break;
        case CHILD_THREAD:
            JNIEnv *ev;
            if(jvm->AttachCurrentThread(&ev,0)!=JNI_OK){
                if(LOG_DEBUG){
                    LOGE("get child thread jnienv wrong");
                }
                return;
            }
            ev->CallVoidMethod(jobj,jm_load,load);
            jvm->DetachCurrentThread();
            break;
    }
}

void YXCallJava::onCallTimeInfo(int threadType,int curr, int total) {
    switch (threadType){
        case MAIN_THREAD:
            jev->CallVoidMethod(jobj,jm_timeinfo,curr,total);
            break;
        case CHILD_THREAD:
            JNIEnv *ev;
            if(jvm->AttachCurrentThread(&ev,0)!=JNI_OK){
                if(LOG_DEBUG){
                    LOGE("get child thread jnienv wrong");
                }
                return;
            }
            ev->CallVoidMethod(jobj,jm_timeinfo,curr,total);
            jvm->DetachCurrentThread();
            break;
    }
}

void YXCallJava::onCallError(int threadType, int code,const char *msg) {
    switch (threadType){
        case MAIN_THREAD: {
            jstring jmsg = jev->NewStringUTF(msg);
            jev->CallVoidMethod(jobj, jm_error, code, jmsg);
            jev->DeleteLocalRef(jmsg);
        }
            break;
        case CHILD_THREAD: {
            JNIEnv *ev;
            if (jvm->AttachCurrentThread(&ev, 0) != JNI_OK) {
                if (LOG_DEBUG) {
                    LOGE("get child thread jnienv wrong");
                }
                return;
            }
            jstring jmsg = ev->NewStringUTF(msg);
            ev->CallVoidMethod(jobj, jm_error, code, jmsg);
            ev->DeleteLocalRef(jmsg);
            jvm->DetachCurrentThread();
        }
            break;
    }
}

void YXCallJava::onCallComplete(int threadType) {
    switch (threadType){
        case MAIN_THREAD:
            jev->CallVoidMethod(jobj,jm_complete);
            break;
        case CHILD_THREAD:
            JNIEnv *ev;
            if(jvm->AttachCurrentThread(&ev,0)!=JNI_OK){
                if(LOG_DEBUG){
                    LOGE("get child thread jnienv wrong");
                }
                return;
            }
            ev->CallVoidMethod(jobj,jm_complete);
            jvm->DetachCurrentThread();
            break;
    }
}

void YXCallJava::onCallValueDB(int threadType, int db) {
    switch (threadType){
        case MAIN_THREAD:
            jev->CallVoidMethod(jobj,jm_valuedb,db);
            break;
        case CHILD_THREAD:
            JNIEnv *ev;
            if(jvm->AttachCurrentThread(&ev,0)!=JNI_OK){
                if(LOG_DEBUG){
                    LOGE("get child thread jnienv wrong");
                }
                return;
            }
            ev->CallVoidMethod(jobj,jm_valuedb,db);
            jvm->DetachCurrentThread();
            break;
    }
}

void YXCallJava::onCallPcmToAAC(int threadType, int size, void *buffer) {
    switch (threadType){
        case MAIN_THREAD: {
            jbyteArray jbuffer = jev->NewByteArray(size);
            jev->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));
            jev->CallVoidMethod(jobj, jm_pcmtoaac, size, jbuffer);
            jev->DeleteLocalRef(jbuffer);
        }
            break;
        case CHILD_THREAD: {
            JNIEnv *ev;
            if (jvm->AttachCurrentThread(&ev, 0) != JNI_OK) {
                if (LOG_DEBUG) {
                    LOGE("get child thread jnienv wrong");
                }
                return;
            }
            jbyteArray jbuffer = ev->NewByteArray(size);
            ev->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));
            ev->CallVoidMethod(jobj, jm_pcmtoaac, size, jbuffer);
            ev->DeleteLocalRef(jbuffer);
            jvm->DetachCurrentThread();
        }
            break;
    }
}

void YXCallJava::onCallPcmInfo(void *buffer, int size) {
    JNIEnv *ev;
    if (jvm->AttachCurrentThread(&ev, 0) != JNI_OK) {
        if (LOG_DEBUG) {
            LOGE("get child thread jnienv wrong");
        }
        return;
    }
    jbyteArray jbuffer = ev->NewByteArray(size);
    ev->SetByteArrayRegion(jbuffer, 0, size, static_cast<const jbyte *>(buffer));
    ev->CallVoidMethod(jobj, jm_pcminfo, jbuffer, size);
    ev->DeleteLocalRef(jbuffer);
    jvm->DetachCurrentThread();
}

void YXCallJava::onCallPcmRate(int samplerate) {
    JNIEnv *ev;
    if(jvm->AttachCurrentThread(&ev,0)!=JNI_OK){
        if(LOG_DEBUG){
            LOGE("get child thread jnienv wrong");
        }
        return;
    }
    ev->CallVoidMethod(jobj,jm_pcmsamplerate,samplerate);
    jvm->DetachCurrentThread();
}
