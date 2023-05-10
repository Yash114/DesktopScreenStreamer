#include "EncoderV2.h"


bool EncoderV2::setup(cv::Mat image) {

	cv::Size MatSize = image.size();

	sws = sws_getContext(1920, 1080, AV_PIX_FMT_RGB0,
		MatSize.width, MatSize.height, AV_PIX_FMT_YUV420P,
		SWS_FAST_BILINEAR, NULL, NULL, NULL);

	if (!sws) {
		printf("Sws Context could not be created!");
		return false;
	}

	x264_param_default_preset(&param, "ultrafast", "zerolatency");
	//param.i_threads = 1;
	param.i_width = MatSize.width;
	param.i_height = MatSize.height;
	param.i_fps_num = 5;
	param.i_fps_den = 1;
	//param.i_keyint_max = 5;
	//param.i_keyint_min = 10;
	x264_param_apply_profile(&param, "high");


	encoder = x264_encoder_open(&param);
	if (!encoder) {
		printf("Encoder cannot be created");
		return false;
	}

	int nheader = 0;

	int r = x264_encoder_headers(encoder, &nals, &nheader);
	if (r < 0) {
		printf("Headers cannot be created");
		return false;
	}

	x264_picture_alloc(&pic_in, X264_CSP_I420, MatSize.width, MatSize.height);


	inLineSize[0] = { 4 * 1920 };


	return true;
}

bool EncoderV2::encode(uint8_t* inputImage, ServerData* output) {

	inData[0] = { inputImage };

	int h = sws_scale(sws, inData, inLineSize, 0,
		1080, pic_in.img.plane, pic_in.img.i_stride);

	pic_in.i_pts = frameIndex;

	int frame_size = x264_encoder_encode(encoder, &nals, &num_nals, &pic_in, &pic_out);
	if (!frame_size) {
		printf("Error!");
		return false;
	}

	frameIndex++;

	output->data = nals[0].p_payload;
	output->size = frame_size;


	return true;
}