#pragma once
#include "Common.h"

class Decoder {
    public:
        bool setup(imgData*, AVCodecID);
        bool decode(Data*, imgData*);
        bool end();

        AVPixelFormat fromPixFormat;
        AVPixelFormat toPixFormat;

    private:
        AVCodec* avcodec = nullptr;
        AVCodecParserContext* parser = nullptr;
        AVCodecContext* avcontext = nullptr;
        AVFrame* tempFrame = nullptr;
        int bytesPerPix = 0;
        SwsContext* context = nullptr;
        int ret = 0;
        AVPacket* pkt = nullptr;
        int cvLinesizes[1];

};


