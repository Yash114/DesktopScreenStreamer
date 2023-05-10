#pragma once
#include "Common.h"

class Encoder {

public:
    bool setup(cv::Mat, AVCodecID);
    bool encode(cv::Mat, AVPacket*);
    bool encode(uint8_t*, AVPacket*);
    bool resize(uint8_t*, ServerData*);
    bool end();
    bool flush(AVPacket*);

    AVPixelFormat fromPixFormat;
    AVPixelFormat toPixFormat;

    AVFrame* avframe = nullptr;
    AVFrame* hwframe = nullptr;


private:

    AVCodec* avcodec = nullptr;
    AVCodecContext* avcontext = nullptr;
    AVPacket* avpkt = nullptr;
    cv::Size size;
    AVPixFmtDescriptor desc;
    int bytesPerPix = 0;
    SwsContext* context = nullptr;

    AVBufferRef* hw_device_ctx;

    int ret = 0;
    int frame = 0;

    uint8_t* inData[1];
    int inLineSize[1];

    int prefixed = 0;
};