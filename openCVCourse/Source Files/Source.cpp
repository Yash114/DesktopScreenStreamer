
#include "Encoder.h"
#include "Decoder.h"
#include "EncoderV2.h"
#include "Server.h"
#include "VideoCapturer.h"
#include "Control.h"

cv::Mat outputMat;

Server mServer;

Encoder mEncoder;
//Decoder mDecoder;

struct imgData ig;
struct imgData* img;

//Data to decoder
//Data* data_to_decoder;
ServerData* outputBuffer;

//Data from encoder
AVPacket* data_from_encoder;
ServerData data_from_encoderV2;

AVPixelFormat fromPixFormat = AV_PIX_FMT_RGB0;
AVPixelFormat toPixFormat = AV_PIX_FMT_YUV420P;

AVCodecID codecs = AV_CODEC_ID_H264;

VideoCapturer videoCapturer;

int val = 0;

bool f = false;
bool* readyToSend = &f;

Mat image(540, 960, CV_8UC3, { 0, 0, 0 });
//Mat image(1080, 1920, CV_8UC3, { 0, 0, 0 });
//Mat image(270, 480, CV_8UC3, { 0, 0, 0 });

void init() {

	//Default Image || Sets size of Decoder

	ig.image = image;
	img = &ig;

	//Build Encoder and decoder from above Mats
	mEncoder.toPixFormat = toPixFormat;
	mEncoder.fromPixFormat = fromPixFormat;
	mEncoder.setup(image, codecs);

	//mDecoder.toPixFormat = toPixFormat;
	//mDecoder.fromPixFormat = fromPixFormat;
	//mDecoder.setup(img, codecs);

	//Sets aside memory for data
	outputBuffer = (ServerData*)malloc(sizeof(ServerData*));

	//data_to_decoder = (Data*)malloc(sizeof(Data*));

	data_from_encoder = av_packet_alloc();
	if (!data_from_encoder)
		exit(1);
}


int frame_count = 0;
bool increment = true;

int connStat = 0;

ImageData* img_recv;
imgData image_got;

EncoderV2 a;


int main() {

	image_got.image = Mat(1080, 1920, CV_8UC3, { 0, 0, 0 });

	a.setup(image);

	int frames = 0;

	int True = 0;
	int False = 1;
	int* done = &False;

	while(true) {
		 
		Connection c = Connection();
		c.start();
		if (c.ready()) {
			mServer = Server();
			mServer.initServer(c);
			connStat = 1;
		}

		//Start image loops
		init();

		img_recv = (struct ImageData*) malloc(sizeof(struct ImageData*));

		if (!videoCapturer.setup()) { return 1; }

		std::thread recieveLoop([] {
			mServer.messageRecvLoop();
		});

		std::thread fpsLoop([] {
			while (true) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				printf("Frame Count: %d\n", frame_count);

				mServer.resendRate = 0;

				frame_count = 0;
			}

		});

		//Captures the video and displays it
		while(mServer.connectionStat == CONNECTED) {
			videoCapturer.getFrame(img_recv);
			a.encode((uint8_t*)img_recv->data, &data_from_encoderV2);
			mServer.SendServerData(&data_from_encoderV2);
			frame_count++;

		}

		recieveLoop.join();

	}

	return 0;
}