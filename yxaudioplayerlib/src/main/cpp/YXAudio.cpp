//
// Created by Administrator on 2019/11/23.
//

#include "YXAudio.h"

YXAudio::YXAudio(YXPlayStatus *yxPlayStatus,int sample_rate,YXCallJava *yxcj) {
   this->yxPlayStatus = yxPlayStatus;
    yxQueue = new YXQueue(yxPlayStatus);
    buffer = static_cast<uint8_t *>(av_malloc(sample_rate * 2 * 2));
    this->sample_rate = sample_rate;
    this->yxCallJava = yxcj;

    //2.初始化对象和内存：
    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(sample_rate * 2 * 2));
    soundTouch = new SoundTouch();
    //3.设置音频数据参数：
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);

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

int YXAudio::reSampleAudio(void **pcmbuf) {
    data_size = 0;
    while (yxPlayStatus != NULL && !yxPlayStatus->exit){
        if( yxPlayStatus->seek)
        {
            continue;
        }
        if(yxQueue->getQueueSize()==0){
           if(!yxPlayStatus->load){
               yxPlayStatus->load = true;
               yxCallJava->onCallLoad(CHILD_THREAD, true);
           }
            continue;
        } else{
            if(yxPlayStatus->load){
                yxPlayStatus->load = false;
                yxCallJava->onCallLoad(CHILD_THREAD, false);
            }
        }

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

             nb = swr_convert(swr_ctx, &buffer, avFrame->nb_samples,
                    (const uint8_t **)(avFrame->data),
                                 avFrame->nb_samples);

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            now_time = avFrame->pts * av_q2d(time_base);
            //fwrite(buffer,1,data_size,outFile);
            if(now_time < clock){
                now_time = clock;
            }
            clock = now_time;
            *pcmbuf = buffer;
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

int YXAudio::getSoundTouchData() {
    while(yxPlayStatus != NULL && !yxPlayStatus->exit)
    {
        out_buffer = NULL;
        if(finished)
        {
            finished = false;
            data_size = reSampleAudio(reinterpret_cast<void **>(&out_buffer));
            if(data_size > 0)
            {
                for(int i = 0; i < data_size / 2 + 1; i++)
                {
                    sampleBuffer[i] = (out_buffer[i * 2] | ((out_buffer[i * 2 + 1]) << 8));
                }
                soundTouch->putSamples(sampleBuffer, nb);
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
            } else{
                soundTouch->flush();
            }
        }
        if(num == 0)
        {
            finished = true;
            continue;
        } else{
            if(out_buffer == NULL)
            {
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                if(num == 0)
                {
                    finished = true;
                    continue;
                }
            }
            return num;
        }
    }
    return 0;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf,void *context){
    YXAudio *yxAudio = (YXAudio *)(context);
    if(yxAudio!=NULL){
        int bufferSize = yxAudio->getSoundTouchData();
        if(bufferSize > 0){
            yxAudio->clock += bufferSize/((double)(yxAudio->sample_rate * 2 * 2));
            if(yxAudio->clock - yxAudio->last_time >= 0.1){
                yxAudio->last_time = yxAudio->clock;
                yxAudio->yxCallJava->onCallTimeInfo(CHILD_THREAD,yxAudio->clock,yxAudio->duration);
            }
             (*yxAudio->pcmBufferQueue)->Enqueue(yxAudio->pcmBufferQueue,(char *) yxAudio->sampleBuffer,bufferSize * 2 * 2);
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

    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE,SL_IID_VOLUME,SL_IID_MUTESOLO};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};
    (*engineEngine)->CreateAudioPlayer(engineEngine,&pcmPlayerObject,&slDataSource,&audioSink,3,ids,req);
    (*pcmPlayerObject)->Realize(pcmPlayerObject,SL_BOOLEAN_FALSE);
    //得到接口后调用,获取player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_PLAY,&pclPlayerPlay);
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_VOLUME,&pcmVolumePlay);
    //获取声道接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_MUTESOLO,&pcmMutePlay);
    //设置缓冲队列和回调函数
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject,SL_IID_BUFFERQUEUE,&pcmBufferQueue);
    setVolume(volumePercent);
    setMute(mute);
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

void YXAudio::pause() {
    if(pcmPlayerObject != NULL){
        (*pclPlayerPlay)->SetPlayState(pclPlayerPlay,SL_PLAYSTATE_PAUSED);
    }
}

void YXAudio::resume() {
    if(pcmPlayerObject != NULL){
        (*pclPlayerPlay)->SetPlayState(pclPlayerPlay,SL_PLAYSTATE_PLAYING);
    }
}

void YXAudio::stop() {
    if(pcmPlayerObject != NULL){
        (*pclPlayerPlay)->SetPlayState(pclPlayerPlay,SL_PLAYSTATE_STOPPED);
    }
}

void YXAudio::release() {
    stop();
    if(yxQueue!=NULL){
        delete(yxQueue);
        yxQueue = NULL;
    }
    if(pcmPlayerObject!=NULL){
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pclPlayerPlay = NULL;
        pcmBufferQueue = NULL;
    }
    if(outputMixObject!=NULL){
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }
    if(engineObject!=NULL){
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    if(buffer!=NULL){
        free(buffer);
        buffer = NULL;
    }
    if(avCodecContext!=NULL){
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }
    if(yxPlayStatus!=NULL){
        yxPlayStatus = NULL;
    }
    if(yxCallJava!=NULL){
        yxCallJava = NULL;
    }
}

void YXAudio::setVolume(int percent) {
    volumePercent = percent;
    if(pcmVolumePlay!=NULL){
        if(percent > 30)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        }
        else if(percent > 25)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        }
        else if(percent > 20)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        }
        else if(percent > 15)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        }
        else if(percent > 10)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        }
        else if(percent > 5)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        }
        else if(percent > 3)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        }
        else if(percent > 0)
        {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        }
        else{
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

void YXAudio::setMute(int mute) {
    this->mute = mute;
   if(pcmMutePlay!=NULL){
       switch (mute){
           case RIGHT_CHANNEL:
               (*pcmMutePlay)->SetChannelMute(pcmMutePlay,1, false);
               (*pcmMutePlay)->SetChannelMute(pcmMutePlay,0, true);
               break;
           case LEFT_CHANNEL:
               (*pcmMutePlay)->SetChannelMute(pcmMutePlay,1, true);
               (*pcmMutePlay)->SetChannelMute(pcmMutePlay,0, false);
               break;
           case STEREO_CHANNEL:
               (*pcmMutePlay)->SetChannelMute(pcmMutePlay,1, false);
               (*pcmMutePlay)->SetChannelMute(pcmMutePlay,0, false);
               break;
       }
   }
}

void YXAudio::setPitch(float pitch) {
    this->pitch = pitch;
    if(soundTouch != NULL)
    {
        soundTouch->setPitch(pitch);
    }
}

void YXAudio::setSpeed(float speed) {
    this->speed = speed;
    if(soundTouch != NULL)
    {
        soundTouch->setTempo(speed);
    }

}


