#include "Encoder.h"

static int set_hwframe_ctx(AVCodecContext* ctx, AVBufferRef* hw_device_ctx)
{
	AVBufferRef* hw_frames_ref;
	AVHWFramesContext* frames_ctx = NULL;
	int err = 0;

	if (!(hw_frames_ref = av_hwframe_ctx_alloc(hw_device_ctx))) {
		fprintf(stderr, "Failed to create VAAPI frame context.\n");
		return -1;
	}
	frames_ctx = (AVHWFramesContext*)(hw_frames_ref->data);
	frames_ctx->format = AV_PIX_FMT_VAAPI;
	frames_ctx->sw_format = AV_PIX_FMT_NV12;
	frames_ctx->width = 1920;
	frames_ctx->height = 1080;
	frames_ctx->initial_pool_size = 20;
	if ((err = av_hwframe_ctx_init(hw_frames_ref)) < 0) {
		fprintf(stderr, "Failed to initialize VAAPI frame context."
			"Error code: %d\n", err);
		av_buffer_unref(&hw_frames_ref);
		return err;
	}
	ctx->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
	if (!ctx->hw_frames_ctx)
		err = AVERROR(ENOMEM);

	av_buffer_unref(&hw_frames_ref);
	return err;
}

bool Encoder::setup(cv::Mat image, AVCodecID codecs) {

	int ret = 0;

	//AVHWDeviceType type = av_hwdevice_find_type_by_name("dxva2");
	//if (type == AV_HWDEVICE_TYPE_NONE) {
	//	std::cout << "Device type cuda is not supported.\n";
	//	std::cout << "Available device types:\n";
	//	while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
	//		std::cout << av_hwdevice_get_type_name(type) << "\n";
	//}

	//hw_device_ctx = av_hwdevice_ctx_alloc(type);
	//if (hw_device_ctx == NULL) {
	//	printf("no!!!\n");
	//}

	//ret = av_hwdevice_ctx_create(&hw_device_ctx, type,
	//	NULL, NULL, 0);
	//if (ret < 0) {
	//	char k[100] = {};
	//	av_make_error_string(k, 100, ret);
	//	fprintf(stderr, "Failed to create a CUDA device. Error code: %s\n", k);
	//	exit(1);
	//}

	avcodec = avcodec_find_encoder(codecs);
	if (!avcodec) {
		std::cout << "Codec with specified id not found" << std::endl;
		exit(1);

	}

	if (!(hwframe = av_frame_alloc())) {
		std::cout << "Connot create hw frame";
		exit(1);
	}

	//Will not work without this
	CoUninitialize();

	avcontext = avcodec_alloc_context3(avcodec);
	if (!avcontext) {
		std::cout << "Can't allocate video codec context" << std::endl;
		exit(1);

	}

	size = image.size();

	avcontext->bit_rate = 50000000;
	avcontext->width = size.width;
	avcontext->height = size.height;
	avcontext->pix_fmt = toPixFormat;

	int frame_rate = 100;

	AVRational timebase;
	timebase.num = 1;
	timebase.den = frame_rate;

	AVRational framerate;
	framerate.num = frame_rate;
	framerate.den = 1;

	avcontext->time_base = timebase;
	avcontext->framerate = framerate;

	avcontext->has_b_frames = 1;
	avcontext->max_b_frames = 10;

	avcontext->compression_level = 0;
	avcontext->delay = 0;
	avcontext->gop_size = 0;
	avcontext->profile = FF_PROFILE_MPEG4_ADVANCED_REAL_TIME;
	avcontext->refs = 0;
	avcontext->thread_count = 0;

	AVDictionary* a = NULL;
	


	avcontext->flags2 = AV_CODEC_FLAG2_FAST;


	ret = avcodec_open2(avcontext, avcodec, NULL);
	if (ret < 0) {
		printf("Could not open codec: %d\n", ret);
		exit(1);
	}



	//if ((ret = av_hwframe_get_buffer(avcontext->hw_frames_ctx, hwframe, 0)) < 0) {
	//	fprintf(stderr, "Error code: %s.\n");
	//	exit(1);
	//}


	avframe = av_frame_alloc();
	if (!avframe) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}

	avframe->format = avcontext->pix_fmt;
	//avframe->format = AV_PIX_FMT_RGB24;
	avframe->width = avcontext->width;
	avframe->height = avcontext->height;

	//Allows for me to put data into a frame for encoding! (Was missing this before)
	ret = av_frame_get_buffer(avframe, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		exit(1);
	}

	//Does this and the above function do the same thing?
	if (av_frame_make_writable(avframe) < 0) {
		std::cerr << "Frame data not writable" << std::endl;
		exit(1);
	}



	context = sws_getContext(
		1920, 1080,
		fromPixFormat,
		avframe->width, avframe->height,
		toPixFormat, 0, nullptr, nullptr, nullptr);

	//context = sws_getContext(
	//	1920, 1080,
	//	fromPixFormat,
	//	avframe->width, avframe->height,
	//	AV_PIX_FMT_RGB24, 0, nullptr, nullptr, nullptr);

	if (!context) {
		std::cerr << "Could not allocate sws context" << std::endl;
		exit(1);
	}


	frame = 0;
	inLineSize[0] = { 4 * 1920 };
	avframe->pts = 0;

	avpkt = av_packet_alloc();


	return true;

}

AVFrame g;
bool Encoder::encode(cv::Mat inputImage, AVPacket* output) {

	ret = 0;
	//actual copy operation
	inData[0] = { inputImage.data };

	//Takes 11ms
	sws_scale(context, inData, inLineSize, 0, 1080, avframe->data, avframe->linesize);

	avframe->pts = frame;
	frame++;

	g = *avframe;

	//encode video frame
	ret = avcodec_send_frame(avcontext, avframe);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		return false;
	}

	//frames may requrue multiple packets to encode an entire frame
	//This cycles through each packet associated with each frame of a video

	while (ret >= 0) {
		ret = avcodec_receive_packet(avcontext, output);
		
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return false;
		}
		else if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}

		return true;
	}

	return true;

}

bool Encoder::encode(uint8_t* inputImage, AVPacket* output) {

	ret = 0;
	//actual copy operation
	inData[0] = { inputImage };

	//Takes 11ms
	sws_scale(context, inData, inLineSize, 0, 1080, avframe->data, avframe->linesize);

	g = *avframe;

	//encode video frame
	ret = avcodec_send_frame(avcontext, avframe);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		return false;
	}

	//frames may requrue multiple packets to encode an entire frame
	//This cycles through each packet associated with each frame of a video

	while (ret >= 0) {
		ret = avcodec_receive_packet(avcontext, output);

		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return false;
		}
		else if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(1);
		}

		avframe->pts = frame;
		frame += 1;

		return true;
	}

	return true;

}

bool Encoder::resize(uint8_t* inputImage, ServerData* outputImage) {

	inData[0] = { inputImage };
	sws_scale(context, inData, inLineSize, 0, 1080, avframe->data, avframe->linesize);
	outputImage->data = avframe->data[0];
	return true;
}

bool Encoder::flush(AVPacket* output){

	//encode video frame
	ret = avcodec_send_frame(avcontext, avframe);
	ret = avcodec_receive_packet(avcontext, output);

	avframe->pts = frame;
	frame++;

	return true;
}

//De allocates all variables
bool Encoder::end() {

	free(context);
	av_free(&avcodec);
	av_free(&avpkt);
	av_free(&avframe);
	av_free(&avcontext);
	av_free(&desc);

	return 1;

}