//
// Created by Administrator on 2019/11/22.
//


#include "YXFFmpeg.h"


YXFFmpeg::YXFFmpeg(YXCallJava *ycjava, const char *url,YXPlayStatus *status) {
    this->yxCallJava = ycjava;
    this->url=url;
    this->status = status;

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
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
              if(yxAudio==NULL){
                  yxAudio = new YXAudio(status,pFormatCtx->streams[i]->codecpar->sample_rate);
                  yxAudio->streamIndex = i;
                  yxAudio->codecpar = pFormatCtx->streams[i]->codecpar;
              }
        }
    }
    //获取解码器
    AVCodec *dec = avcodec_find_decoder(yxAudio->codecpar->codec_id);
    if(!dec){
        if(LOG_DEBUG){
            LOGE("can not find decoder");
        }
        return;
    }
    //利用解码器创建解码器上下文
    yxAudio->avCodecContext = avcodec_alloc_context3(dec);
    if(!yxAudio->avCodecContext){
        if(LOG_DEBUG){
            LOGE("can not alloc new decoderctx");
        }
        return;
    }

    if(avcodec_parameters_to_context(yxAudio->avCodecContext,yxAudio->codecpar)<0){
        if(LOG_DEBUG){
            LOGE("can not fill decoderctx");
        }
        return;
    }
    //打开解码器
    if(avcodec_open2(yxAudio->avCodecContext,dec,0)!=0){
        if(LOG_DEBUG){
            LOGE("can not open audio streams");
        }
        return;
    }
    yxCallJava->onCallPrepared(CHILD_THREAD);
}

void YXFFmpeg::start() {

    if(yxAudio==NULL){
        if(LOG_DEBUG){
            LOGE("yxaudio is null");
        }
        return;
    }
    yxAudio->play();
    int count = 0;
    while(status!=NULL && !status->exit){
        //读取音频帧
        AVPacket *packet = av_packet_alloc();
        if(av_read_frame(pFormatCtx,packet)==0){
            if(packet->stream_index == yxAudio->streamIndex){
                count++;
                if(LOG_DEBUG){
                    LOGE("解码第 %d 帧",count);
                }
                yxAudio->yxQueue->putAvpacket(packet);
            } else{
                av_packet_free(&packet);
                av_free(packet);
                packet = NULL;
            }
        } else{
           av_packet_free(&packet);
           av_free(packet);
           packet = NULL;
           while(status!=NULL && !status->exit){
                if(yxAudio->yxQueue->getQueueSize() > 0){
                    continue;
                } else{
                    status->exit = true;
                    break;
                }
           }
        }
    }
    if(LOG_DEBUG){
        LOGD("解码完成");
    }
}


