//
// Created by Administrator on 2019/11/23.
//

#include "YXAudio.h"

YXAudio::YXAudio(YXPlayStatus *yxPlayStatus,int sample_rate) {
   this->yxPlayStatus = yxPlayStatus;
    yxQueue = new YXQueue(yxPlayStatus);
    buffer = static_cast<uint8_t *>(av_malloc(44100 * 2 * 2));
    this->sample_rate = sample_rate;
}

YXAudio::~YXAudio() {
}

void *decodePlay(void *data){
    YXAudio *yxAudio = static_cast<YXAudio *>(data);
    yxAudio->initOpenSLES();
    pthread_exit(&yxAudio->thread_play);
}

void YXAudio::play() {
    pthread_create(&thread_play,NULL,decodePlay,this);
}

 //FILE *outFile = fopen("/sdcard/ffmpegmusic.pcm","w");

int YXAudio::reSampleAudio() {
    while (yxPlayStatus != NULL && !yxPlayStatus->exit){
        avPacket = av_packet_alloc();
        if(yxQueue->getAvpacket(avPacket)!=0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        ret = avcodec_send_packet(avCodecContext,avPacket);
        if(ret != 0){
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext,avFrame);
        if(ret == 0){
            if(avFrame->channels > 0 && avFrame->channel_layout == 0){
                //根据声道数返回声道布局
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if(avFrame->channels == 0 && avFrame->channel_layout > 0){
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }
            SwrContext *swr_ctx;
            swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    static_cast<AVSampleFormat>(avFrame->format),
                    avFrame->sample_rate,
                    NULL,NULL);

            if(!swr_ctx || swr_init(swr_ctx) < 0){
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                if(swr_ctx != NULL){
                    swr_free(&swr_ctx);
                    swr_ctx = NULL;
                }
                continue;
            }

            int nb = swr_convert(swr_ctx, &buffer, avFrame->nb_samples,
                    (const uint8_t **)(avFrame->data),
                                 avFrame->nb_samples);

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

            //fwrite(buffer,1,data_size,outFile);
            if(LOG_DEBUG){
                LOGE("data size is %d",data_size);
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            swr_ctx = NULL;
            break;
        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }
   // fclose(outFile);
    return data_size;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf,void *context){
    YXAudio *yxAudio = (YXAudio *)(context);
    if(yxAudio!=NULL){
        int bufferSize = yxAudio->reSampleAudio();

        if(bufferSize > 0){
            (*yxAudio->pcmBufferQueue)->Enqueue(yxAudio->pcmBufferQueue,yxAudio->buffer,bufferSize);
        }
    }
}

void YXAudio::initOpenSLES() {

    //创建引擎
    slCreateEngine(&engineObject,0,0,0,0,0);
    //实现引擎接口对象
    (*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    //通过engineObject的getInterface方法初始化engineEngine
    (*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineEngine);

    //设置混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    (*engineEngine)->CreateOutputMix(engineEngine,&outputMixObject,1,mids,mreq);
    (*outputMixObject)->Realize(outputMixObject,SL_BOOLEAN_FALSE);
    (*outputMixObject)->GetInterface(outputMixObject,SL_IID_ENVIRONMENTALREVERB,&outputMixEnvironmentalReverb);
    (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb,&reverbSettings);
    //创建播放器
    SLDataLocator_AndroidBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            2,
            (SLuint32)getCurrentSampleRateForOpensles(sample_rate),
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX,outputMixObject};
    SLDataSource slDataSource = {&android_queue,&pcm};
    SLDataSink audioSink = {&outputMix,NULL};

    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    (*engineEngine)->CreateAudioPlayer(engineEngine,&pcmPlayerObject,&slDataSource,&audioSink,1,ids,req);
    (*pcmPlayerObject)->Realize(pcmPlayerObject,SL_BOOLEAN_FALSE);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_PLAY,&pclPlayerPlay);

    //设置缓冲队列和回调函数
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_BUFFERQUEUE,&pcmBufferQueue);
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue,pcmBufferCallBack,this);
    (*pclPlayerPlay)->SetPlayState(pclPlayerPlay,SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue,this);

}

int YXAudio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate){
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}
