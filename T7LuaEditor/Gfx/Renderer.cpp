#include "./Renderer.h"
#include "GfxResource/Viewport.h"
#include "GfxResource/VertexShader.h"
#include "GfxResource/PixelShader.h"
#include "GfxResource/VertexBuffer.h"


#define TEST_TEXTURE TEXT("C:/Users/coxtr/Downloads/the-shard-x0-3840x2160.jpg")

Renderer::Renderer(HWND _hwnd, size_t _width, size_t _height)
        : hwnd{_hwnd}, width{_width}, height{_height}, frameTimer{} {
    SetupDx11();
    backgroundColor_ = {
            0.1,
            0.1,
            0.1,
            1.0
    };
    imgui_ = std::make_unique<ImGUIManager>(device.Get(), context.Get(), hwnd);
    demoTexture = LoadTexture(device.Get(), context.Get(), TEST_TEXTURE);
}

Renderer::~Renderer() {
}

void Renderer::CreateRenderTarget() {
    ID3D11Texture2D *pBackBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    device->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
    pBackBuffer->Release();
}

void Renderer::CleanupRenderTarget() {
    renderTargetView.Reset();
}

void Renderer::Render() {
    frameTimer.Start();

    imgui_->RenderUI();
    context->OMSetRenderTargets(1,
                                renderTargetView.GetAddressOf(),
                                nullptr);

    context->ClearRenderTargetView(renderTargetView.Get(), (float*)&backgroundColor_);

    DrawTestImage();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swapChain->Present(1, 0);
}

void Renderer::Resize(LPARAM lParam, WPARAM wParam) {
    if (device && wParam != SIZE_MINIMIZED) {
        CleanupRenderTarget();
        swapChain->ResizeBuffers(0,
                                 (UINT) LOWORD(lParam),
                                 (UINT) HIWORD(lParam),
                                 DXGI_FORMAT_UNKNOWN,
                                 0);
        CreateRenderTarget();
    }
}

bool Renderer::SetupDx11() {
    /*
        * 1. create the swapchain, the device, the context, and render targets
        * 2. set up shaders (vertex shader and then pixel shader)
        * 3. create viewports
        * 4. create constant buffer
        * 5. setup ortho projection
        * 6. finally create a fucking triangle
        * 7. cry
    */

    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0,
                                                    D3D_FEATURE_LEVEL_10_0,};
    HRESULT res = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createDeviceFlags,
            featureLevelArray,
            2,
            D3D11_SDK_VERSION,
            &sd,
            swapChain.GetAddressOf(),
            device.GetAddressOf(),
            &featureLevel,
            context.GetAddressOf());

    assert(SUCCEEDED(res));

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
    const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
    device->CreateBlendState(&alphaBlend, &blendState_);
    context->OMSetBlendState(blendState_.Get(), blend_factor, 0xffffffff );
    // setup raster state
    D3D11_RASTERIZER_DESC rasterDesc{};
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

    res = device->CreateRasterizerState(&rasterDesc, rasterizerState.GetAddressOf());
    assert(SUCCEEDED(res));

    context->RSSetState(rasterizerState.Get());

    // setup viewport
    viewport.Height = (float) height;
    viewport.Width = (float) width;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0;
    context->RSSetViewports(1, &viewport);

    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = 16;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    device->CreateBuffer(&bd, nullptr, &constantBuf_);
    // setup matrices

    float fov = DirectX::XM_PIDIV4;
    float aspectRatio = viewport.Width / viewport.Height;
    constexpr float ZNEAR = 1.0f;
    constexpr float ZFAR = 650.0f;
    projectionMat = DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, ZNEAR, ZFAR);
    worldMat = DirectX::XMMatrixIdentity();
    orthoMat = DirectX::XMMatrixOrthographicLH(viewport.Width, viewport.Height, ZNEAR, ZFAR);
    CreateRenderTarget();

    return true;
}


void Renderer::DrawTestImage()
{
    constexpr auto ShaderPath = L"C:/Users/coxtr/source/repos/T7LuaEditor/T7LuaEditor/ShaderSrc/test.hlsl";
    static ConstantBuffer cbuf{1.0f};
    cbuf.time += 1.f;

    static VertexShader vShader( *this, ShaderPath );
    static PixelShader pShader( *this, ShaderPath);
    static std::vector<Vertex> triangle = {
            {DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
            {DirectX::XMFLOAT3(0.45f, -0.5, 0.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},
            {DirectX::XMFLOAT3(-0.45f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)}
    };
    static std::vector<Vertex> triangle2 = {
            {DirectX::XMFLOAT3(0.3f, 0.7f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f)},
            {DirectX::XMFLOAT3(0.45f, -0.5, 0.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)},
            {DirectX::XMFLOAT3(-0.45f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)}
    };
    static VertexBuffer vertices(*this, triangle);
    static VertexBuffer vertices2(*this, triangle2);
    context->UpdateSubresource(constantBuf_.Get(), 0, 0, &cbuf, 0, 0);

    context->PSSetConstantBuffers(0, 1, constantBuf_.GetAddressOf());
    context->VSSetConstantBuffers(0, 1, constantBuf_.GetAddressOf());


    vShader.Bind( *this );
    pShader.Bind( *this );

    D3D11_INPUT_ELEMENT_DESC ied[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    ID3D11InputLayout* layout;
    device->CreateInputLayout( ied, 2, vShader.GetBufferPtr(), vShader.GetProgramSize(), &layout );
    context->IASetInputLayout(layout);
    vertices.Bind(*this);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->Draw(3, 0);
    vertices2.Bind(*this);
    context->Draw(3, 0);
}







