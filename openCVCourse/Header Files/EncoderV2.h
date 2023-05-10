#pragma once

#include "Common.h"

class EncoderV2
{

	public: 
		bool setup(cv::Mat);
		bool encode(uint8_t*, ServerData*);

		int frameIndex = 0;

	private:

		x264_t* encoder;
		SwsContext* sws;
		x264_param_t param;
		
		uint8_t* inData[1];
		int inLineSize[1];

		x264_picture_t pic_in;
		x264_picture_t pic_out;

		x264_nal_t* nals;
		int num_nals = 0;
};

