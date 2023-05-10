#pragma once
#include "Common.h"

#include <dxgi1_2.h>
#include <d3d11.h>
#include <d2d1_1.h>
#include <vector>
#include <Windows.Foundation.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")

using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace std;
using namespace cv;

class VideoCapturer
{

	public:
		int setup();
		void start(ImageData*, int*);
		void getFrame(ImageData*);

		void end();

		bool ready = false;
		bool running = false;
		bool imgReady = false;


	private:
		Mat capturedImage;
	
		ID3D11Device* _lDevice;
		ID3D11DeviceContext* _lImmediateContext;
		IDXGIOutputDuplication* _lDeskDupl = NULL;
		ID3D11Texture2D* _lAcquiredDesktopImage;
		DXGI_OUTPUT_DESC _lOutputDesc;
		DXGI_OUTDUPL_DESC _lOutputDuplDesc;
		ID3D11Texture2D* currTexture;

		D3D11_TEXTURE2D_DESC desc;
		D3D11_MAPPED_SUBRESOURCE _resource;

		DXGI_OUTDUPL_DESC lOutputDuplDesc;

		IDXGIFactory2* DXGIfactory = NULL;
		IDXGIAdapter1* DXGIadaper = NULL;
		IDXGIDevice1* pDXGIDevice;
		IDXGIOutput* lDxgiOutput;
		IDXGIOutput1* lDxgiOutput1;
		DXGI_ADAPTER_DESC1 d;


		D3D_DRIVER_TYPE DriverTypes[4] = {
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
			D3D_DRIVER_TYPE_UNKNOWN
		};

		// Feature levels supported
		D3D_FEATURE_LEVEL gFeatureLevels[6] = {
		  D3D_FEATURE_LEVEL_11_0,
		  D3D_FEATURE_LEVEL_10_1,
		  D3D_FEATURE_LEVEL_10_0,
		  D3D_FEATURE_LEVEL_9_3,
		  D3D_FEATURE_LEVEL_9_2,
		  D3D_FEATURE_LEVEL_9_1,
		};
		UINT gNumFeatureLevels = ARRAYSIZE(gFeatureLevels);
		D3D_FEATURE_LEVEL lFeatureLevel;

};

