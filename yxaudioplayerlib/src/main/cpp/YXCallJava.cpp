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
