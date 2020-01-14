//
// Created by Administrator on 2020/1/8.
//

#include "YXVideo.h"

YXVideo::YXVideo(YXPlayStatus *yxPlayStatus, YXCallJava *yxCallJava) {
    //在构造方法中将以下声明的进行初始化
    this->playStatus = yxPlayStatus;
    this->callJava = yxCallJava;
    this->queue = new YXQueue(yxPlayStatus);


}

YXVideo::~YXVideo() {

}

void *playVideo(void *data){
    //获取YXVideo对象
    YXVideo *video = static_cast<YXVideo *>(data);
    //在存活状态下循环解码操作
    while(video->playStatus!=NULL && !video->playStatus->exit){
          //seek时,不执行其他操作
          if(video->playStatus->seek){
              av_usleep(1000 * 100);//优化消耗
              continue;
          }
          //如果队列当中没有AVpacket,也不执行以下解码操作
          if(video->queue->getQueueSize() == 0){
              if(!video->playStatus->load){
                  //如果不是load状态就告诉用户,并置为load状态
                  video->playStatus->load = true;
                  video->callJava->onCallLoad(CHILD_THREAD, true);
              }
              av_usleep(1000 * 100);//优化消耗
              continue;
          } else{
              //有的话，开始解码
              //这个时候要将load置为false,并通知用户
              if(video->playStatus->load){
                  video->playStatus->load = false;
                  video->callJava->onCallLoad(CHILD_THREAD, false);
              }
          }
        //获得一个AVpacket(分配内存空间)
        AVPacket *avPacket = av_packet_alloc();
        //如果取不到就不往下执行了
        if(video->queue->getAvpacket(avPacket)!=0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        //将拿到的AVpacket发送到FFmpeg内部(将原始数据包数据作为输入提供给解码器。)
        if(avcodec_send_packet(video->avCodecContext,avPacket)!=0){
            //如果输入失败,则停止当次循环，并释放AVpacket
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        //获得一个AVframe(分配内存空间)
        AVFrame *avFrame = av_frame_alloc();
        //从解码器返回解码的输出数据。
        if(avcodec_receive_frame(video->avCodecContext,avFrame)!=0){
            //如果数据返回失败,则释放,并退出当次循环
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        LOGE("子线程解码一个AVframe成功");
        //如果AVframe的视频格式为YUV420P则直接丢给应用层渲染,否则先转成YUV420P格式
        if(avFrame->format == AV_PIX_FMT_YUV420P){//直接渲染

            video->callJava->onCallRendererYUV(
                    avFrame->linesize[0],
                    avFrame->height,
                    avFrame->data[0],
                    avFrame->data[1],
                    avFrame->data[2]);
        } else{//转换格式
            LOGE("当前视频不是YUV420P格式");
            //得到一个AVframe(需要分配内存空间)
            AVFrame *pFrameYUV420P = av_frame_alloc();
            //返回以字节为单位的大小，以存储数据所需的数据量
            int size = av_image_get_buffer_size(
                    AV_PIX_FMT_YUV420P,
                    avFrame->linesize[0],
                    avFrame->height,
                    1);
            //获取一个要存储的buffer空间
            uint8_t *buffer = static_cast<uint8_t *>(av_malloc(size * sizeof(uint8_t)));
            //根据指定的图像设置数据指针和行大小(填充数据)
            av_image_fill_arrays(
                    pFrameYUV420P->data,
                    pFrameYUV420P->linesize,
                    buffer,
                    AV_PIX_FMT_YUV420P,
                    avFrame->linesize[0],
                    avFrame->height,
                    1);
            //分配并返回SwsContext。 您需要它来执行转换操作
            SwsContext *sws_ctx = sws_getContext(
                    avFrame->linesize[0],
                    avFrame->height,
                    video->avCodecContext->pix_fmt,
                    avFrame->linesize[0],
                    avFrame->height,
                    AV_PIX_FMT_YUV420P,
                    SWS_BICUBIC,NULL,NULL,NULL);
            if(!sws_ctx){
                //返回失败，释放资源
                av_frame_free(&pFrameYUV420P);
                av_free(pFrameYUV420P);
                av_free(buffer);
                continue;
            }
            //进行转换
            sws_scale(
                    sws_ctx,
                    reinterpret_cast<const uint8_t *const *>(avFrame->data),
                    avFrame->linesize,
                    0,
                    avFrame->height,
                    pFrameYUV420P->data,
                    pFrameYUV420P->linesize);

            //回调到应用层渲染
            video->callJava->onCallRendererYUV(
                    avFrame->linesize[0],
                    avFrame->height,
                    pFrameYUV420P->data[0],
                    pFrameYUV420P->data[1],
                    pFrameYUV420P->data[2]);

            av_frame_free(&pFrameYUV420P);
            av_free(pFrameYUV420P);
            av_free(buffer);
            sws_freeContext(sws_ctx);
        }

        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }
    //最后退出线程
    pthread_exit(&video->thread_play);
}

void YXVideo::play() {
    //创建线程
    pthread_create(&thread_play,NULL,playVideo,this);
}

void YXVideo::release() {
    //释放队列
    if(queue!=NULL){
        delete(queue);
        queue = NULL;
    }
    //释放上下文
    if(avCodecContext != NULL){
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    if(playStatus!=NULL){
        playStatus = NULL;
    }
    if(callJava!=NULL){
        callJava = NULL;
    }
}

