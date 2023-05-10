#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <combaseapi.h>
#include <string.h>

#include <x264.h>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/opt.h>
	#include <libavutil/imgutils.h>
	#include <libswscale/swscale.h>
	#include <libavutil/pixdesc.h>
    #include <libavutil/hwcontext.h>

	#include <errno.h>
}

#define FAIL 1
#define SUCCESS 0

using namespace cv;
using namespace std;

struct Data {
	uint8_t* data = nullptr;
	int size = 0;
};

struct ImageData {
	void* data;
	int rowPitch;
};

struct ServerData {
	uint8_t* data = nullptr;
	int size = 0;
};

struct imgData {
	Mat image;
};