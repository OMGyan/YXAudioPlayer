//
// Created by Administrator on 2019/11/22.
//


#include "YXFFmpeg.h"


YXFFmpeg::YXFFmpeg(YXCallJava *ycjava, const char *url) {
    this->yxCallJava = ycjava;
    this->url=url;

}

void *decodeFFmpeg(void *data){
    YXFFmpeg *yxfFmpeg = static_cast<YXFFmpeg *>(data);
    yxfFmpeg->decodeFFmpegThread();
    pthread_exit(&yxfFmpeg->decodeThread);
}

void YXFFmpeg::prepared() {
   pthread_create(&decodeThread,NULL,decodeFFmpeg,this);
}

void YXFFmpeg::decodeFFmpegThread() {
    //注册解码器并初始化网络
   av_register_all();
   avformat_network_init();
   //打开文件或网络流
   pFormatCtx = avformat_alloc_context();
   if(avformat_open_input(&pFormatCtx,url,NULL,NULL) != 0){
       if(LOG_DEBUG){
           LOGE("can not open url :%s",url);
       }
       return;
   }
   //获取流信息
   if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
        if(LOG_DEBUG){
            LOGE("can not find stream from url :%s",url);
        }
        return;
   }
   //循环获取音频流
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        
    } 
}
