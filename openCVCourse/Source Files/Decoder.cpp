#include "Decoder.h"
    bool Decoder::setup(imgData* outImg, AVCodecID codecs) {

        avcodec = avcodec_find_decoder(codecs);
        if (!avcodec) {
            fprintf(stderr, "Codec not found\n");
            exit(1);
        }

        parser = av_parser_init(avcodec->id);
        if (!parser) {
            fprintf(stderr, "parser not found\n");
            exit(1);
        }

        avcontext = avcodec_alloc_context3(avcodec);
        if (!avcontext) {
            fprintf(stderr, "Could not allocate video codec context\n");
            exit(1);
        }

        cv::Size size = outImg->image.size();

        avcontext->width = size.width;
        avcontext->height = size.height;

        if (avcodec->id == AV_CODEC_ID_H264)
            av_opt_set(avcontext->priv_data, "preset", "slow", 0);

        if (avcodec_open2(avcontext, avcodec, NULL) < 0) {
            fprintf(stderr, "Could not open codec\n");
            exit(1);
        }

        tempFrame = av_frame_alloc();
        if (!tempFrame) {
            fprintf(stderr, "Could not allocate video frame\n");
            exit(1);
        }

        tempFrame->format = fromPixFormat;
        tempFrame->width = size.width;
        tempFrame->height = size.height;

        outImg->image = Mat(size.width, size.height, CV_8UC3);

        ret = 0;

        ret = av_frame_get_buffer(tempFrame, 0);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate the video frame data\n");
            exit(1);
        }

        //Does this and the above function do the same thing?
        if (av_frame_make_writable(tempFrame) < 0) {
            std::cerr << "Frame data not writable" << std::endl;
            exit(1);
        }

        //Create data packet
       pkt = av_packet_alloc();

        return true;
    }

    bool Decoder::decode(Data* data, imgData* outImg) {

        fflush(stdout);
        ret = 0;

        printf("%d\n", data->data[data->size - 1]);

        while (data->size > 0) {

            ret = av_parser_parse2(parser, avcontext, &pkt->data, &pkt->size,
                data->data, data->size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);

            if (ret < 0) {
                fprintf(stderr, "Error while parsing\n");
                exit(1);
            }

            if (pkt->size) {

                ret = avcodec_send_packet(avcontext, pkt);
                if (ret < 0) {
                    printf("Error sending a packet for decoding %d\n", ret);
                    exit(1);
                }

                while (ret >= 0) {
                    ret = avcodec_receive_frame(avcontext, tempFrame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;

                    }
                    else if (ret < 0) {
                        fprintf(stderr, "Error during decoding\n");
                        exit(1);
                    }

                    break;
                }

                if (context == nullptr) {

                    const int width3 = avcontext->width * 3;

                    cvLinesizes[0] = { width3 };

                    context = sws_getContext(
                        avcontext->width, avcontext->height, 
                        toPixFormat,
                        avcontext->width, avcontext->height,
                        AV_PIX_FMT_RGB24, 0, NULL, NULL, NULL);
                }

                int y = sws_scale(context, tempFrame->data, tempFrame->linesize, 0, 
                    avcontext->height, &outImg->image.data, cvLinesizes);

                return true;

            }
        }
        return false;

    }

    bool Decoder::end() {


        free(context);
        av_free(&avcodec);
        av_free(&avcontext);

        return 1;

    }

