#include "./renderer.h"
#include "gpu_resources.h"
#include "shader_util.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <Tracy.hpp>

Renderer::Renderer(HWND _hwnd, float _width, float _height) : hwnd{_hwnd}, width{_width}, height{_height}
{
    initialize_d3d();
    initialize_imgui();
    clearColor = {0.0f, 0.05490196078431372549019607843137f, 0.14117647058823529411764705882353f, 1.0f};
}

Renderer::~Renderer() = default;

void Renderer::create_backbuffer_view()
{
    ID3D11Texture2D *pBackBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    device->CreateRenderTargetView(pBackBuffer, nullptr, &rtv);
    pBackBuffer->Release();
}

void Renderer::reset_backbuffer_views()
{
    rtv.Reset();
}

void Renderer::resize_swapchain_backbuffer(int newWidth, int newHeight, bool minimized)
{
    if (device && !minimized)
    {
        reset_backbuffer_views();
        swapChain->ResizeBuffers(0, newWidth, newHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        width = newWidth;
        height = newHeight;
        viewport.Width = width;
        viewport.Height = height;
        create_backbuffer_view();
    }
}

bool Renderer::initialize_d3d()
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
    res = device->CreateRasterizerState2(&rasterDesc, rasterizerState.GetAddressOf());
    assert(SUCCEEDED(res));
    context->RSSetState(rasterizerState.Get());

    viewport.Height = (float)height;
    viewport.Width = (float)width;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0;
    context->RSSetViewports(1, &viewport);

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

    create_backbuffer_view();
    return true;
}

void Renderer::set_and_clear_backbuffer()
{
    ZoneScoped("clearbb");
    context->RSSetViewports(1, &viewport);
    context->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);
    context->ClearRenderTargetView(rtv.Get(), (float *)&clearColor);
}

void Renderer::present()
{
    ZoneScoped("present");
    swapChain->Present(1, 0);
}

bool Renderer::initialize_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplDX11_Init(device.Get(), context.Get());
    ImGui_ImplWin32_Init(hwnd);
    return true;
}

void Renderer::imgui_frame_begin()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Renderer::imgui_frame_end()
{
    ZoneScoped("imgui_end");
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

// void Renderer::create_render_pass_resources(char* passKey, PassDependencies deps, RenderPassResources* outPass)
// {
//     auto dev = device.Get();
//     build_vertex_shader_and_input_layout(dev,
//             deps.vertexShaderPath,
//             deps.il,
//             deps.ilSize,
//             &outPass->vertexShader,
//             &outPass->vertexInputLayout);

//     build_pixel_shader(dev, deps.pixelShaderPath, &outPass->pixelShader);
//     create_constant_buffer(dev, deps.constantBufferSize, &outPass->constantBuffer);
//     create_dynamic_vertex_buffer(dev, &outPass->vertexBuffer, deps.vertexBufferSize);
//     create_dynamic_index_buffer(dev, &outPass->indexBuffer, deps.indexBufferSize);
//     alloc_texture_atlas(deps.atlasSlot, &outPass->textureAtlas);
// }
