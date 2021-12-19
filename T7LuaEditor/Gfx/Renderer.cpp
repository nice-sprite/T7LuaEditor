#include "./Renderer.h"
#include "GfxResource/Viewport.h"
#include "GfxResource/VertexShader.h"
#include "GfxResource/PixelShader.h"
#include "GfxResource/VertexBuffer.h"
#include <fmt/core.h>
#include <fmt/format.h>

#define TEST_TEXTURE TEXT("C:/Users/coxtr/Downloads/integra.jpg")

Renderer::Renderer(HWND _hwnd, size_t _width, size_t _height)
        : hwnd{_hwnd}, width{_width}, height{_height}, frameTimer{} {
    InitializeDx11();
    backgroundColor_ = {
            0.1,
            0.1,
            0.1,
            1.0
    };
    imgui_ = std::make_unique<ImGUIManager>(device_.Get(), ctx_.Get(), hwnd);
    demoTexture = LoadTexture(device_.Get(), ctx_.Get(), TEST_TEXTURE);
}

Renderer::~Renderer() = default;

void Renderer::CreateRenderTarget() {
    ID3D11Texture2D *pBackBuffer;
    swapChain_->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    device_->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView_);
    pBackBuffer->Release();
    SetupD2D();
}

void Renderer::CleanupRenderTarget() {
    renderTargetView_.Reset();
    renderTarget2d_.Reset();
}

void Renderer::Render() {
    frameTimer.Start();
    ctx_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);
    ctx_->ClearRenderTargetView(renderTargetView_.Get(), (float *) &backgroundColor_);
    imgui_->BeginUI();
    if (ImGui::Begin("debug")) {
        ImGui::Text("viewport_: %f x %f", viewport_.Width, viewport_.Height);
        wrl::ComPtr<ID3D11Texture2D> backBuffer;
        D3D11_TEXTURE2D_DESC desc;

        swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        backBuffer->GetDesc(&desc);
        ImGui::Text("backbuffer: %d x %d", desc.Width, desc.Height);
    }
    ImGui::End();

    DrawTestImage();
    renderTarget2d_->BeginDraw();
    using namespace std::string_literals;
    std::wstring text = L"fps: "s;
    text += std::to_wstring(ImGui::GetIO().Framerate);
    renderTarget2d_->DrawTextW(text.c_str(), text.length(), textFormat_.Get(),
                               D2D1::RectF(0, 0, (float) viewport_.Width, (float) viewport_.Height), whiteBrush_.Get());
    renderTarget2d_->EndDraw();
    imgui_->EndUI();
    swapChain_->Present(1, 0);
}

void Renderer::Resize(LPARAM lParam, WPARAM wParam) {
    if (device_ && wParam != SIZE_MINIMIZED) {
        CleanupRenderTarget();
        swapChain_->ResizeBuffers(0,
                                  (UINT) LOWORD(lParam),
                                  (UINT) HIWORD(lParam),
                                  DXGI_FORMAT_B8G8R8A8_UNORM,
                                  0);
        width = (UINT) LOWORD(lParam);
        height = (UINT) HIWORD(lParam);
        viewport_.Width = width;
        viewport_.Height = height;

        CreateRenderTarget();
    }
}

bool Renderer::InitializeDx11() {

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_12_0,
                                               D3D_FEATURE_LEVEL_11_1,
                                               D3D_FEATURE_LEVEL_11_0};
    wrl::ComPtr<ID3D11Device> baseDevice;
    wrl::ComPtr<ID3D11DeviceContext> baseCtx;

    HRESULT res = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevels,
                                    _countof(featureLevels), D3D11_SDK_VERSION, &baseDevice, &featureLevel, &baseCtx);
    assert(SUCCEEDED(res));

    if (SUCCEEDED(baseDevice.As(&device_))) {
        baseCtx.As(&ctx_); // upgrade context to revision 4
    }

    {
        wrl::ComPtr<IDXGIDevice4> dxgiDevice;
        wrl::ComPtr<IDXGIFactory> dxgiBaseFact;

        if (SUCCEEDED(device_.As(&dxgiDevice))) {
            wrl::ComPtr<IDXGIAdapter> adapter;
            if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter))) {
                HRESULT hr = adapter->GetParent(IID_PPV_ARGS(&dxgiBaseFact));
                if (SUCCEEDED(hr)) {
                    dxgiBaseFact.As(&dxgiFactory_); // get the factory that created the d3d11 device
                }
            }
        }
    }

#pragma region setup swapChain
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd = {0};
    sd.Width = width;
    sd.Height = height;
    sd.BufferCount = 2;
    sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    wrl::ComPtr<IDXGISwapChain1> baseSwapChain;
    dxgiFactory_->CreateSwapChainForHwnd((IUnknown *) device_.Get(), hwnd, &sd, nullptr, nullptr, &baseSwapChain);
    baseSwapChain.As(&swapChain_); // upgrade the swapchain to revision 4
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
    device_->CreateBlendState(&alphaBlend, &blendState_);
    ctx_->OMSetBlendState(blendState_.Get(), blend_factor, 0xffffffff);
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

    res = device_->CreateRasterizerState2(&rasterDesc, rasterizerState_.GetAddressOf());
    assert(SUCCEEDED(res));

    ctx_->RSSetState(rasterizerState_.Get());
#pragma endregion

#pragma region setup viewport
    viewport_.Height = (float) height;
    viewport_.Width = (float) width;
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
    viewport_.TopLeftX = 0.0f;
    viewport_.TopLeftY = 0.0;
    ctx_->RSSetViewports(1, &viewport_);
#pragma endregion

#pragma region create constant buffer
    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 64;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device_->CreateBuffer(&bd, nullptr, &constantBuf_);
#pragma  endregion

#pragma region setup matrices
    float fov = DirectX::XM_PIDIV4;
    float aspectRatio = viewport_.Width / viewport_.Height;
    constexpr float ZNEAR = 1.0f;
    constexpr float ZFAR = 100.0f;
    projectionMat_ = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.f), aspectRatio, ZNEAR, ZFAR);
    worldMat_ = DirectX::XMMatrixIdentity();
    orthoMat_ = DirectX::XMMatrixOrthographicLH(viewport_.Width, viewport_.Height, ZNEAR, ZFAR);
    viewMat_ = DirectX::XMMatrixLookAtLH(
            DirectX::XMVECTOR{0.0, 0.0f, 2.f, 0.f},
            DirectX::XMVECTOR{0.0f, 0.0f, 0.0f, 0.f},
            DirectX::XMVECTOR{0.f, 1.f, 0.f, 0.f});
#pragma endregion

    CreateRenderTarget();

    return true;
}

bool Renderer::SetupD2D() {
    HRESULT res;
    D2D1_FACTORY_OPTIONS opts{};
    wrl::ComPtr<ID2D1Factory7> factory2d;
    wrl::ComPtr<ID3D11Texture2D> backBuffer;
    wrl::ComPtr<IDXGIDevice4> dxgiDevice;
    wrl::ComPtr<IDXGISurface> dxgiBackbuffer;

    opts.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
    res = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, opts, factory2d.GetAddressOf());
    assert(SUCCEEDED(res));
    res = device_.As(&dxgiDevice);
    assert(SUCCEEDED(res));

    res = factory2d->CreateDevice(dxgiDevice.Get(), &device2d_);
    assert(SUCCEEDED(res));

    res = device2d_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &context2d_);
    assert(SUCCEEDED(res));
    res = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory7), &writeFactory_);
    assert(SUCCEEDED(res));

    swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    res = backBuffer.As(&dxgiBackbuffer);
    assert(SUCCEEDED(res));
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_HARDWARE,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
            0,
            0
    );

    res = factory2d->CreateDxgiSurfaceRenderTarget(dxgiBackbuffer.Get(), props, &renderTarget2d_);
    assert(SUCCEEDED(res));

    writeFactory_->CreateTextFormat(
            L"Iosevka",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            16,
            L"",
            &textFormat_
    );
    renderTarget2d_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &whiteBrush_);
    return true;
}

void Renderer::DrawTestImage() {
    DirectX::XMMATRIX rotation{};
    static float time = 0.f;
    time += 0.01f;
    rotation = DirectX::XMMatrixRotationZ(time);
    constexpr auto ShaderPath = L"C:/Users/coxtr/source/repos/T7LuaEditor/T7LuaEditor/ShaderSrc/test.hlsl";
    static ConstantBuffer cbuf{};
    cbuf.modelViewProjection = rotation * viewMat_ * projectionMat_;

    static VertexShader vShader(*this, ShaderPath);
    static PixelShader pShader(*this, ShaderPath);
    static std::vector<Vertex> texturedRect = {
            {DirectX::XMFLOAT3(-0.5f, -0.5f, 0.f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), {1.f, 1.f}},
            {DirectX::XMFLOAT3(0.5f, -0.5f, 0.f),  DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), {0.f, 1.f}},
            {DirectX::XMFLOAT3(-0.5f, 0.5f, 0.f),  DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), {1.f, 0.f}},
            {DirectX::XMFLOAT3(0.5f, 0.5f, 0.f),   DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), {0.f, 0.f}}
    };


    static VertexBuffer vertices(*this, texturedRect);
    ctx_->UpdateSubresource(constantBuf_.Get(), 0, 0, &cbuf, 0, 0);

    ctx_->PSSetShaderResources(0, 1, demoTexture.view_.GetAddressOf());
    ctx_->PSSetConstantBuffers(0, 1, constantBuf_.GetAddressOf());
    ctx_->VSSetConstantBuffers(0, 1, constantBuf_.GetAddressOf());


    vShader.Bind(*this);
    pShader.Bind(*this);

    DWORD idx[] = {
            0, 1, 2, 2, 1, 3
    };

    D3D11_BUFFER_DESC id;
    // create the index buffer
    id.Usage = D3D11_USAGE_DYNAMIC;
    id.ByteWidth = sizeof(DWORD) * 6;
    id.BindFlags = D3D11_BIND_INDEX_BUFFER;
    id.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    id.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA indexData{};
    indexData.pSysMem = idx;
    ID3D11Buffer *indexBuffer_;
    device_->CreateBuffer(&id, &indexData, &indexBuffer_);

    ctx_->IASetIndexBuffer(indexBuffer_, DXGI_FORMAT_R32_UINT, 0);


    D3D11_INPUT_ELEMENT_DESC ied[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    ID3D11InputLayout *layout;
    device_->CreateInputLayout(ied, 3, vShader.GetBufferPtr(), vShader.GetProgramSize(), &layout);
    ctx_->IASetInputLayout(layout);
    vertices.Bind(*this);
    ctx_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx_->DrawIndexed(6, 0, 0);
}







