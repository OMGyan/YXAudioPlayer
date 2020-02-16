//
// Created by Administrator on 2019/12/30.
//

#include "YXPcmBean.h"

YXPcmBean::YXPcmBean(SAMPLETYPE *buffer, int size) {
    //根据大小初始化缓冲内存空间
    this->buffer = (char *)malloc(size);
    this->buffsize = size;
    //把传进来的buffer拷贝到上面新分配的内存当中
    memcpy(this->buffer,buffer,size);
}

YXPcmBean::~YXPcmBean() {
    //进行资源释放
    free(buffer);
    buffer = NULL;
}
