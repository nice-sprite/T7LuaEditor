#include "./Renderer.h"
#include "gpu_resources.h"

#define TEST_TEXTURE TEXT("C:/Users/coxtr/Downloads/integra.jpg")
#define GRID_TEXTURE TEXT("C:/Users/coxtr/source/repos/T7LuaEditor/Resource/Textures/grid_64x64.png")

#include <random>
float RandUniformRange(float min_, float max_)
{
    static std::default_random_engine rng;
    static std::uniform_real_distribution<float> dist(min_, max_);
    return dist(rng);
}

Renderer::Renderer(HWND _hwnd, float _width, float _height) : hwnd{_hwnd}, width{_width}, height{_height}, frameTimer{}
{
    Init();
    clearColor = {0.0f, 0.05490196078431372549019607843137f, 0.14117647058823529411764705882353f, 1.0f};


    // std::vector<TexturedQuad> quads;
    // std::vector<Vertex> verts;
    // std::vector<DWORD> indexs;
    // 20,000/128 = 157 shader resource updates assuming every single quad uses a different texture
    // - Reserved Texture Slots:
    // - 0: $white
    // - 1: $black
    // - 2: $grid

    // D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT = 14
    // Assuming all 20,000 elements have unique transform:
    // the biggest constant buffer D3D11 supports is 4096 4*32bit constants (float4)
    // this means we can have 4096/4 = 1024 transform matrices in one constant buffer.
    // - assume we have 20,000 transforms, one per each quad. This requires 20 constant buffer uploads per frame
    // for (int i = 0; i < 2; ++i)
    // {
    //     auto xOffset = RandUniformRange(-2.f, 2.f);
    //     auto yOffset = RandUniformRange(-0.9f, 0.9f);
    //     xOffset = 0.f;
    //     yOffset = 0.f;
    //     auto r = RandUniformRange(0.f, 1.f);
    //     auto g = RandUniformRange(0.f, 1.f);
    //     auto b = RandUniformRange(0.f, 1.f);
    //     auto a = RandUniformRange(0.1f, 1.f);
    //     TexturedQuad myRect = {
    //         -1280.f / 2.0f + xOffset,
    //         1280.f / 2.0f + xOffset,
    //         -720.f / 2.0f + yOffset,
    //         720.f / 2.0f + yOffset,
    //         r, g, b, a};

    //     quads.push_back(myRect);

    //     verts.push_back(Vertex{XMFLOAT3(myRect.left, myRect.top, 0.0f), XMFLOAT4(r, g, b, a), XMFLOAT2(1.f, 1.f)});     // left top
    //     verts.push_back(Vertex{XMFLOAT3(myRect.right, myRect.top, 0.0f), XMFLOAT4(r, g, b, a), XMFLOAT2(0.f, 1.f)});    // right top
    //     verts.push_back(Vertex{XMFLOAT3(myRect.left, myRect.bottom, 0.0f), XMFLOAT4(r, g, b, a), XMFLOAT2(1.f, 0.f)});  // left bottom
    //     verts.push_back(Vertex{XMFLOAT3(myRect.right, myRect.bottom, 0.0f), XMFLOAT4(r, g, b, a), XMFLOAT2(0.f, 0.f)}); // right bottom

    //     int vertNum = i * 4;
    //     indexs.push_back(0 + (vertNum));
    //     indexs.push_back(1 + (vertNum));
    //     indexs.push_back(2 + (vertNum));
    //     indexs.push_back(2 + (vertNum));
    //     indexs.push_back(1 + (vertNum));
    //     indexs.push_back(3 + (vertNum));
    // }
}

Renderer::~Renderer() = default;

void Renderer::CreateRenderTarget()
{
    ID3D11Texture2D *pBackBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    device->CreateRenderTargetView(pBackBuffer, nullptr, &rtv);
    pBackBuffer->Release();
    SetupD2D();
}

void Renderer::CleanupRenderTarget()
{
    rtv.Reset();
    rtv2D.Reset();
}

void Renderer::Resize(LPARAM lParam, WPARAM wParam)
{
    if (device && wParam != SIZE_MINIMIZED)
    {
        CleanupRenderTarget();
        swapChain->ResizeBuffers(0,
                                 (UINT)LOWORD(lParam),
                                 (UINT)HIWORD(lParam),
                                 DXGI_FORMAT_B8G8R8A8_UNORM,
                                 0);
        width = (float)(UINT)LOWORD(lParam);
        height = (float)(UINT)HIWORD(lParam);
        viewport.Width = width;
        viewport.Height = height;
        CreateRenderTarget();
    }
}

bool Renderer::Init()
{
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_12_0,
                                               D3D_FEATURE_LEVEL_11_1,
                                               D3D_FEATURE_LEVEL_11_0};
    ComPtr<ID3D11Device> baseDevice;
    ComPtr<ID3D11DeviceContext> baseCtx;

    HRESULT res = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevels,
                                    _countof(featureLevels), D3D11_SDK_VERSION, &baseDevice, &featureLevel, &baseCtx);
    assert(SUCCEEDED(res));

    if (SUCCEEDED(baseDevice.As(&device)))
    {
        baseCtx.As(&context); // upgrade context to revision 4
    }

    {
        ComPtr<IDXGIDevice4> dxgiDevice;
        ComPtr<IDXGIFactory> dxgiBaseFact;

        if (SUCCEEDED(device.As(&dxgiDevice)))
        {
            ComPtr<IDXGIAdapter> adapter;
            if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter)))
            {
                HRESULT hr = adapter->GetParent(IID_PPV_ARGS(&dxgiBaseFact));
                if (SUCCEEDED(hr))
                {
                    dxgiBaseFact.As(&dxgiFactory); // get the factory that created the d3d11 device
                }
            }
        }
    }

#pragma region setup swapChain
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd = {0};
    sd.Width = static_cast<UINT>(width);
    sd.Height = static_cast<UINT>(height);
    sd.BufferCount = 2;
    sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ComPtr<IDXGISwapChain1> baseSwapChain;
    dxgiFactory->CreateSwapChainForHwnd((IUnknown *)device.Get(), hwnd, &sd, nullptr, nullptr, &baseSwapChain);
    baseSwapChain.As(&swapChain); // upgrade the swapchain to revision 4
#pragma endregion

#pragma region setup AlphaBlend state
    D3D11_BLEND_DESC alphaBlend{};
    alphaBlend.RenderTarget[0].BlendEnable = true;
    alphaBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    alphaBlend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    alphaBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    alphaBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    alphaBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    alphaBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    alphaBlend.RenderTarget[0].RenderTargetWriteMask = 0x0f;

    // Setup blend state
    const float blend_factor[4] = {0.f, 0.f, 0.f, 0.f};
    device->CreateBlendState(&alphaBlend, &alphaBlendState);
    context->OMSetBlendState(alphaBlendState.Get(), blend_factor, 0xffffffff);
#pragma endregion

#pragma region setup rasterizer state
    // setup raster state
    D3D11_RASTERIZER_DESC2 rasterDesc{};
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;

    res = device->CreateRasterizerState2(&rasterDesc, rasterizerState_.GetAddressOf());
    assert(SUCCEEDED(res));

    context->RSSetState(rasterizerState_.Get());
#pragma endregion

#pragma region setup viewport
    viewport.Height = (float)height;
    viewport.Width = (float)width;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0;
    context->RSSetViewports(1, &viewport);
#pragma endregion

#pragma region create sampler state for grid

    // TODO: move to CommonStates.h/cpp
    D3D11_SAMPLER_DESC gridSampler{};
    gridSampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    gridSampler.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    gridSampler.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    gridSampler.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    gridSampler.MipLODBias = 0;
    gridSampler.MaxAnisotropy = 1;
    gridSampler.ComparisonFunc = D3D11_COMPARISON_NEVER;
    gridSampler.BorderColor[0] = 1.f;
    gridSampler.BorderColor[1] = 1.f;
    gridSampler.BorderColor[2] = 1.f;
    gridSampler.BorderColor[3] = 1.f;
    gridSampler.MinLOD = -FLT_MAX;
    gridSampler.MaxLOD = FLT_MAX;

    device->CreateSamplerState(&gridSampler, &gridSS);

#pragma endregion

    CreateRenderTarget();

    return true;
}

bool Renderer::SetupD2D()
{
    HRESULT res;
    D2D1_FACTORY_OPTIONS opts{};
    ComPtr<ID2D1Factory7> factory2d;
    ComPtr<ID3D11Texture2D> backBuffer;
    ComPtr<IDXGIDevice4> dxgiDevice;
    ComPtr<IDXGISurface> dxgiBackbuffer;

    opts.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    res = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, opts, factory2d.GetAddressOf());
    assert(SUCCEEDED(res));
    res = device.As(&dxgiDevice);
    assert(SUCCEEDED(res));

    res = factory2d->CreateDevice(dxgiDevice.Get(), &device2D);
    assert(SUCCEEDED(res));

    res = device2D->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &context2D);
    assert(SUCCEEDED(res));
    res = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory7), &DWriteFactory);
    assert(SUCCEEDED(res));

    swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    res = backBuffer.As(&dxgiBackbuffer);
    assert(SUCCEEDED(res));
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_HARDWARE,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
        0,
        0);

    res = factory2d->CreateDxgiSurfaceRenderTarget(dxgiBackbuffer.Get(), props, &rtv2D);
    assert(SUCCEEDED(res));

    DWriteFactory->CreateTextFormat(
        L"Iosevka",
        nullptr,
        DWRITE_FONT_WEIGHT_LIGHT,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        16,
        L"",
        &defaultDWTextFormat);
    rtv2D->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &solidWhiteBrush2D);
    return true;
}

void Renderer::ClearRTV()
{
    context->RSSetViewports(1, &viewport);
    context->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);
    context->ClearRenderTargetView(rtv.Get(), (float *)&clearColor);
    rtv2D->BeginDraw();
}

void Renderer::Present()
{
    rtv2D->EndDraw();
    swapChain->Present(1, 0);
}

float Renderer::GetWidth() const
{
    return width;
}

float Renderer::GetHeight() const
{
    return height;
}

ID3D11Device5 *Renderer::GetDevice() const noexcept
{
    return device.Get();
}

ID3D11DeviceContext *Renderer::GetContext() const noexcept
{
    return context.Get();
}