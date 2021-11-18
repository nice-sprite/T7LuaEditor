#include "./Gfx.h"


ID3D11Device* Gfx::g_pDevice = nullptr;
IDXGISwapChain* Gfx::g_pSwapChain = nullptr;
ID3D11DeviceContext* Gfx::g_pContext = nullptr;
ID3D11RenderTargetView* Gfx::g_pRenderTargetView = nullptr;
ID3D11VertexShader* Gfx::g_pVertexShader = nullptr;
ID3D11InputLayout* Gfx::g_pVertexLayout = nullptr;
ID3D11PixelShader* Gfx::g_pPixelShader = nullptr;
ID3D11Buffer* Gfx::g_pVertexBuffer = nullptr;
ID3D11Buffer* Gfx::g_pIndexBuffer = nullptr;
ID3D11Buffer* Gfx::g_pConstantBuffer = nullptr;
D3D11_VIEWPORT Gfx::g_pViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{ 0 };
DirectX::XMMATRIX Gfx::mOrtho{};
HWND Gfx::g_hwnd = NULL;


bool Gfx::CompileShader_Mem(const char* szShader,
    const char* szEntrypoint,
    const char* szTarget,
    ID3D10Blob** pBlob )
{
    ID3D10Blob* pErrorBlob = nullptr;

    auto hr = D3DCompile( szShader, strlen( szShader ), 0, nullptr, nullptr, szEntrypoint, szTarget, D3DCOMPILE_ENABLE_STRICTNESS, 0, pBlob, &pErrorBlob );
    if (FAILED( hr ))
    {
        if (pErrorBlob)
        {
            char szError[256]{ 0 };
            memcpy( szError, pErrorBlob->GetBufferPointer(), pErrorBlob->GetBufferSize() );
            MessageBoxA( nullptr, szError, "Error", MB_OK );
        }
        return false;
    }
    return true;

}

void Gfx::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer; 
    g_pSwapChain->GetBuffer( 0, IID_PPV_ARGS( &pBackBuffer ) );
    g_pDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
}

void Gfx::CleanupRenderTarget()
{
    if (g_pRenderTargetView) { g_pRenderTargetView->Release(); g_pRenderTargetView = NULL; }
}

void Gfx::Render()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    bool demo = true;
    ImGui::ShowDemoWindow( &demo );

    ImGui::Render();
    const float clearColor[4] = { 
        0.5,
        0.5,
        0.5,
        1.0
    };

    g_pContext->OMSetRenderTargets( 1, &g_pRenderTargetView, NULL );

    g_pContext->ClearRenderTargetView( g_pRenderTargetView, clearColor );
    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
    g_pSwapChain->Present( 1, 0 );
}

void Gfx::Resize()
{
}

bool Gfx::StartDx11(HWND hWnd)
{
    /*
        * 1. create the swapchain, the device, the context, and render targets
        * 2. set up shaders (vertex shader and then pixel shader)
        * 3. create viewports
        * 4. create constant buffer
        * 5. setup ortho projection
        * 6. finally create a fucking triangle
        * 7. cry
    */
    g_hwnd = hWnd;
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain( NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        createDeviceFlags,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &g_pSwapChain,
        &g_pDevice,
        &featureLevel,
        &g_pContext );

    if (res != S_OK)
        return false;


    CreateRenderTarget();

    return true;
}

void Gfx::PrepareImGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init( g_hwnd );
    ImGui_ImplDX11_Init( g_pDevice, g_pContext );
}

void Gfx::ShutdownDx11()
{
    CleanupRenderTarget();
    SafeRelease( &g_pSwapChain );
    SafeRelease( &g_pContext);
    SafeRelease( &g_pDevice );
}

