#include "./Gfx.h"
#include "GfxResource/Viewport.h"

#include <ImSequencer.h>

Gfx::Gfx(HWND _hwnd, size_t _width, size_t _height)
        : hwnd{_hwnd}, width{_width}, height{_height}, mainTimer{} {
    SetupDx11();
    PrepareImGui();
}


Gfx::~Gfx() {
    ShutdownImGui();
    ShutdownDx11();
}


void Gfx::CreateRenderTarget() {
    ID3D11Texture2D *pBackBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    device->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
    pBackBuffer->Release();
}

void Gfx::CleanupRenderTarget() {
    renderTargetView.Reset();
}

void Gfx::Render() {
    static long double totalElapsedMs;
    mainTimer.Start();
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    static bool demo = true;
    ImGui::ShowDemoWindow(&demo);
    static bool showSystemInfo = true;
    if (ImGui::Begin("System Info", &showSystemInfo)) {
        for (auto const& deviceInfoStr: systemInfo.ToString()) {
            ImGui::Text(deviceInfoStr.c_str());
        }
    }
    ImGui::End();
    auto timeElapsedMs = mainTimer.Stop() * .001;
    totalElapsedMs += timeElapsedMs;
    auto timeStr = fmt::format("{}ms\ntotal: {}ms", timeElapsedMs, totalElapsedMs);
    ImGui::GetForegroundDrawList()->AddText(ImVec2(0, 0), 0xFFFFFFFF, timeStr.c_str());
    ImGui::Render();
    const float clearColor[4] = {
            0.1,
            0.1,
            0.1,
            1.0
    };

    context->OMSetRenderTargets(1,
                                renderTargetView.GetAddressOf(),
                                nullptr);

    context->ClearRenderTargetView(renderTargetView.Get(), clearColor);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swapChain->Present(1, 0);
}

void Gfx::Resize(LPARAM lParam, WPARAM wParam) {
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

bool Gfx::SetupDx11() {
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

void Gfx::PrepareImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device.Get(), context.Get());
}

void Gfx::ShutdownDx11() {
}

void Gfx::ShutdownImGui() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}






