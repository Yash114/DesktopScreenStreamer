#include "VideoCapturer.h"

int VideoCapturer::setup() {

    HRESULT hr(E_FAIL);

    hr = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)&DXGIfactory);
    if (FAILED(hr) || DXGIfactory == NULL) {
        printf("failed to create factory: %d", hr);
        return false;
    }

    hr = DXGIfactory->EnumAdapters1(0, &DXGIadaper);
    if (FAILED(hr) || DXGIadaper == NULL) {
        printf("failed to create directory: %d", hr);
        return false;
    }


    UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

    for (UINT DriverTypeIndex = 0; DriverTypeIndex < 4; ++DriverTypeIndex)
    {
        hr = D3D11CreateDevice(DXGIadaper, DriverTypes[DriverTypeIndex],
            nullptr, 0, gFeatureLevels, gNumFeatureLevels, D3D11_SDK_VERSION,
            &_lDevice, &lFeatureLevel, &_lImmediateContext);

        if (SUCCEEDED(hr) && _lDevice != NULL) {
            break;
        }
    }

    if (FAILED(hr) || _lDevice == NULL) {
        printf("bfailed to create 3d3 device\n");
        std::cout << std::hex << hr;

        return false;
    }


    DXGIadaper->GetDesc1(&d);
    printf("%ws\n", d.Description);

    hr = _lDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&pDXGIDevice);
    if (FAILED(hr) || pDXGIDevice == NULL) {
        std::cout << "On Query Interface\n";
        std::cout << std::hex << hr;
        return false;
    }

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    hr = DXGIadaper->EnumOutputs(0, &lDxgiOutput);
    if (FAILED(hr)) {
        std::cout << "On Enum Outputs\n";
        std::cout << std::hex << hr;
        return false;
    }

    // QI for Output 1
    hr = lDxgiOutput->QueryInterface(__uuidof(lDxgiOutput1), reinterpret_cast<void**>(&lDxgiOutput1));
    lDxgiOutput->Release();
    lDxgiOutput = nullptr;
    if (FAILED(hr)) {
        std::cout << "On Query Interface from output\n";
        std::cout << std::hex << hr;
        return false;
    }

    // Create desktop duplication
    hr = lDxgiOutput1->DuplicateOutput(_lDevice, &_lDeskDupl);
   
    if (FAILED(hr)) {
        std::cout << "On Duplicate Output\n";
        std::cout << std::hex << hr;
        return false;
    }


    ready = true;

    return true;
}

HRESULT hr(E_FAIL);
IDXGIResource* deskRes = nullptr;
DXGI_OUTDUPL_FRAME_INFO frameInfo;

ID3D11Texture2D* gpuTex = nullptr;
ID3D11Texture2D* cpuTex = nullptr;

D3D11_MAPPED_SUBRESOURCE sr;

void VideoCapturer::getFrame(ImageData* imageDataOut) {

        _lDeskDupl->AcquireNextFrame(100, &frameInfo, &deskRes);

        if (deskRes != NULL) {

            deskRes->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&gpuTex);
            deskRes->Release();
            deskRes = nullptr;

            gpuTex->GetDesc(&desc);
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
            desc.Usage = D3D11_USAGE_STAGING;
            desc.BindFlags = 0;
            desc.MiscFlags = 0; // D3D11_RESOURCE_MISC_GDI_COMPATIBLE ?

            imgReady = false;

            if (running)
                cpuTex->Release();

            _lDevice->CreateTexture2D(&desc, nullptr, &cpuTex);

            _lImmediateContext->CopyResource(cpuTex, gpuTex);

            gpuTex->Release();
            gpuTex = nullptr;

            _lImmediateContext->Map(cpuTex, 0, D3D11_MAP_READ, 0, &sr);
            _lImmediateContext->Unmap(cpuTex, 0);

            imageDataOut->data = sr.pData;
            imageDataOut->rowPitch = sr.RowPitch;

            imgReady = true;

            _lDeskDupl->ReleaseFrame();

            if (!running)
                running = true;


        }
}

void VideoCapturer::end() {

    _lDeskDupl->Release();
    ready = false;
    running = false;

}