//
// Created by Administrator on 2019/11/22.
//



#include "YXFFmpeg.h"


YXFFmpeg::YXFFmpeg(YXCallJava *ycjava, const char *url,YXPlayStatus *status) {
    this->yxCallJava = ycjava;
    this->url=url;
    this->status = status;
    pthread_mutex_init(&init_mutex,NULL);
    pthread_mutex_init(&seek_mutex,NULL);
}

void *decodeFFmpeg(void *data){
    YXFFmpeg *yxfFmpeg = static_cast<YXFFmpeg *>(data);
    yxfFmpeg->decodeFFmpegThread();
    pthread_exit(&yxfFmpeg->decodeThread);
}

void YXFFmpeg::prepared() {
   pthread_create(&decodeThread,NULL,decodeFFmpeg,this);
}

int avformat_callback(void *ctx){
   YXFFmpeg *yxfFmpeg = (YXFFmpeg *)ctx;
   if(yxfFmpeg->status->exit){
       return AVERROR_EOF;
   }
    return 0;
}

void YXFFmpeg::decodeFFmpegThread() {

    pthread_mutex_lock(&init_mutex);
    //注册解码器并初始化网络
   av_register_all();
   avformat_network_init();
   //打开文件或网络流
   pFormatCtx = avformat_alloc_context();

   pFormatCtx->interrupt_callback.callback = avformat_callback;
   pFormatCtx->interrupt_callback.opaque = this;

   if(avformat_open_input(&pFormatCtx,url,NULL,NULL) != 0){
       if(LOG_DEBUG){
           LOGE("can not open url :%s",url);
           yxCallJava->onCallError(CHILD_THREAD,CAN_NOT_OPEN_URL,"can not open url");
       }
       exit = true;
       pthread_mutex_unlock(&init_mutex);
       return;
   }
   //获取流信息
   if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
        if(LOG_DEBUG){
            LOGE("can not find stream from url :%s",url);
            yxCallJava->onCallError(CHILD_THREAD,CAN_NOT_FIND_STREAM_FROM_URL,"can not find stream from url");
        }
       exit = true;
       pthread_mutex_unlock(&init_mutex);
        return;
   }
   //循环获取音频流
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){//得到音频流
              if(yxAudio==NULL){
                  yxAudio = new YXAudio(status,pFormatCtx->streams[i]->codecpar->sample_rate,yxCallJava);
                  yxAudio->streamIndex = i;
                  yxAudio->codecpar = pFormatCtx->streams[i]->codecpar;
                  //总时长(秒)
                  yxAudio->duration = pFormatCtx->duration / AV_TIME_BASE;
                  yxAudio->time_base = pFormatCtx->streams[i]->time_base;
                  duration = yxAudio->duration;
                  yxCallJava->onCallPcmRate(yxAudio->sample_rate);
              }
        } else if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){//得到视频流
              if(video==NULL){
                  //创建yxvideo
                  video = new YXVideo(status,yxCallJava);
                  //给视频索引赋值
                  video->videoStreamIndex = i;
                  //给视频解码器属性赋值
                  video->avCodecParameters = pFormatCtx->streams[i]->codecpar;
                  //给时间刻度赋值
                  video->time_base = pFormatCtx->streams[i]->time_base;

                  int num = pFormatCtx->streams[i]->avg_frame_rate.num;
                  int den = pFormatCtx->streams[i]->avg_frame_rate.den;

                  if(den!=0 && num!=0){
                      int fps = num / den;
                      video->defaultDelayTime = 1.0 / fps;
                  }

              }
        }
    }
    if(yxAudio!=NULL){
        getCodecContext(yxAudio->codecpar,&yxAudio->avCodecContext);
    }
    if(video!=NULL){
        //拿到解码器上下文
        getCodecContext(video->avCodecParameters,&video->avCodecContext);
    }
    yxCallJava->onCallPrepared(CHILD_THREAD);
    pthread_mutex_unlock(&init_mutex);

}

void YXFFmpeg::start() {

    if(yxAudio==NULL){
        if(LOG_DEBUG){
            LOGE("yxaudio is null");
            yxCallJava->onCallError(CHILD_THREAD,YXAUDIO_IS_NULL,"yxaudio is null");
        }
        return;
    }
    if(video == NULL){
        if(LOG_DEBUG){
            yxCallJava->onCallError(CHILD_THREAD,YXAUDIO_IS_NULL,"yxvidio is null");
        }
        return;
    }
    video->audio = yxAudio;

    yxAudio->play();
    //启动视频线程
    video->play();
    while(status!=NULL && !status->exit){
        //seek下不解码
        if(status->seek){
            av_usleep(1000 * 100);
            continue;
        }
        if(yxAudio->yxQueue->getQueueSize() > 10){
            av_usleep(1000 * 100);
            continue;
        }
        //读取数据帧
        AVPacket *packet = av_packet_alloc();
        pthread_mutex_lock(&seek_mutex);
        int ret = av_read_frame(pFormatCtx,packet);
        pthread_mutex_unlock(&seek_mutex);
        if(ret==0){
            if(packet->stream_index == yxAudio->streamIndex) {
                yxAudio->yxQueue->putAvpacket(packet);
            } else if(packet->stream_index == video->videoStreamIndex){
               //根据视频流索引将相应AVpacket放入队列
                video->queue->putAvpacket(packet);
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
                    av_usleep(1000 * 100);
                    continue;
                } else{
                    status->exit = true;
                    break;
                }
           }
            break;
        }
    }
    if(yxCallJava!=NULL){
        yxCallJava->onCallComplete(CHILD_THREAD);
    }
    exit = true;

}

void YXFFmpeg::pause() {
   if(yxAudio!=NULL){
       yxAudio->pause();
   }
}

void YXFFmpeg::resume() {
    if(yxAudio!=NULL){
        yxAudio->resume();
    }
}

void YXFFmpeg::release() {
    /*if(status->exit){
        return;
    }*/
    status->exit = true;
    pthread_mutex_lock(&init_mutex);
    int sleepCount = 0;
    while(!exit){
        if(sleepCount>1000){
            exit = true;
        }
        if(LOG_DEBUG){
            LOGE("wait ffmpeg exit %d",sleepCount);
        }
        sleepCount++;
        av_usleep(1000 * 10);
    }
    if(yxAudio!=NULL){
        yxAudio->release();
        delete(yxAudio);
        yxAudio = NULL;
    }
    LOGE("释放video");
    if(video != NULL){
        video->release();
        delete(video);
        video = NULL;
    }
    if(pFormatCtx!=NULL){
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
    }
    if(status!=NULL){
        status = NULL;
    }
    if(yxCallJava!=NULL){
        yxCallJava = NULL;
    }
    pthread_mutex_unlock(&init_mutex);
}

YXFFmpeg::~YXFFmpeg() {
   pthread_mutex_destroy(&init_mutex);
   pthread_mutex_destroy(&seek_mutex);
}

void YXFFmpeg::seek(int64_t secds) {
    //如果duration不大于0,直接退出
    if(!(duration>0)){
        return;
    }
    //seek范围是duration之内
    if(secds >=0 && secds <= duration){
        if(yxAudio!=NULL){
            //开始进行seek
            status->seek = true;
            //清空队列
            yxAudio->yxQueue->clearAvpacket();
            //时间重置
            yxAudio->clock = 0;
            yxAudio->last_time = 0;
            //由于seek过程涉及AVFormatContext所以得加锁
            pthread_mutex_lock(&seek_mutex);
            //获取seek的时间,并进行seek
            int64_t rel = secds * AV_TIME_BASE;
            /**
             * 问题:seek时播放不连贯
             * 原因:
             * 由于一个AVPacket里面有多个AVFrame,当seek时,FFmpeg解码器
             * 中还残留AVFrame,所以导致seek后，不能立即播放当前音乐
             */
            avcodec_flush_buffers(yxAudio->avCodecContext);
            avformat_seek_file(pFormatCtx,-1,INT64_MIN,rel,INT64_MAX,0);
            pthread_mutex_unlock(&seek_mutex);
            status->seek = false;

        }
    }
}

void YXFFmpeg::setVolume(int percent) {
    if(yxAudio!=NULL){
        yxAudio->setVolume(percent);
    }
}

void YXFFmpeg::setMute(int mute) {
    if(yxAudio!=NULL){
        yxAudio->setMute(mute);
    }
}

void YXFFmpeg::setPitch(float pitch) {
    if(yxAudio!=NULL){
        yxAudio->setPitch(pitch);
    }

}

void YXFFmpeg::setSpeed(float speed) {
    if(yxAudio!=NULL){
        yxAudio->setSpeed(speed);
    }
}

int YXFFmpeg::getSampleRate() {
    if(yxAudio!= NULL){
        return yxAudio->avCodecContext->sample_rate;
    }
    return 0;
}

void YXFFmpeg::startStopRecord(bool start) {
   if(yxAudio!=NULL){
       yxAudio->startStopRecord(start);
   }
}

bool YXFFmpeg::cutAudio(int start_time, int end_time, bool isShowPcm) {
    if(start_time >= 0 && end_time <= duration && start_time < end_time){
        yxAudio->isCut = true;
        yxAudio->end_time = end_time;
        yxAudio->showPcm = isShowPcm;
        seek(start_time);
        return true;
    }
    return false;
}

int YXFFmpeg::getCodecContext(AVCodecParameters *codecParameters, AVCodecContext **codecContext) {
    //获取解码器
    AVCodec *dec = avcodec_find_decoder(codecParameters->codec_id);
    if(!dec){
        if(LOG_DEBUG){
            LOGE("can not find decoder");
            yxCallJava->onCallError(CHILD_THREAD,CAN_NOT_FIND_DECODER,"can not find decoder");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    //利用解码器创建解码器上下文
    *codecContext = avcodec_alloc_context3(dec);
    if(!yxAudio->avCodecContext){
        if(LOG_DEBUG){
            LOGE("can not alloc new decoderctx");
            yxCallJava->onCallError(CHILD_THREAD,CAN_NOT_ALLOC_DECODER_CONTEXT,"can not alloc new decoderctx");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    if(avcodec_parameters_to_context(*codecContext,codecParameters)<0){
        if(LOG_DEBUG){
            LOGE("can not fill decoderctx");
            yxCallJava->onCallError(CHILD_THREAD,CAN_NOT_FILL_DECODER_CONTEXT,"can not fill decoderctx");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    //打开解码器
    if(avcodec_open2(*codecContext,dec,0)!=0){
        if(LOG_DEBUG){
            LOGE("can not open audio streams");
            yxCallJava->onCallError(CHILD_THREAD,CAN_NOT_OPEN_AUDIO_STREAMS,"can not open audio streams");
        }
        exit = true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }
    return 0;
}


