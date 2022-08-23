#include "./renderer.h"
#include "gpu_resources.h"
#include "shader_util.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <Tracy.hpp>
#include "win32_input.h"

/*
 * Helper functions for debugging
*/

void print_vec(char* label, XMVECTOR v) {
    char buff[1024]{};
    strcat(buff, label);
    strcat(buff, "%f %f %f %f");
    ImGui::Text(buff, 
        XMVectorGetX(v), 
        XMVectorGetY(v), 
        XMVectorGetZ(v), 
        XMVectorGetW(v)
    );
}


Renderer::Renderer(HWND _hwnd, float _width, float _height) : 
    hwnd{_hwnd}, 
    width{_width}, 
    height{_height}, 
    camera(DirectX::XM_PIDIV2, 16.0f/9.0f, 10.0f, 2000.0f),
    debug_lines{},
    debug_line_count{0}
{
    initialize_d3d();
    initialize_imgui();
    clearColor = { 0.f, 0.f, 0.f, 1.0f}; // pitch black

    /*create scene resources*/
    HRESULT buff_r = create_dynamic_vertex_buffer(device.Get(), 
        scene_vertex_buffer.GetAddressOf(), 
        sizeof(VertexPosColorTexcoord) * 10000
    );

    if(!SUCCEEDED(buff_r)) {
        // debug
        __debugbreak();
    }

    buff_r = create_dynamic_index_buffer(
        device.Get(), 
        scene_index_buffer.GetAddressOf(), 
        10000 * 6
    );

    if(!SUCCEEDED(buff_r)) {
        __debugbreak();
    }

    build_vertex_shader(
        device.Get(), 
        DEFAULT_SHADER, 
        scene_vertex_shader.GetAddressOf(), 
        vtx_pos_color_tex_il.GetAddressOf()
    );

    build_pixel_shader(
        device.Get(), 
        DEFAULT_SHADER,
        scene_pixel_shader.GetAddressOf()
    );

    create_constant_buffer(
        device.Get(),
        sizeof(PerSceneConsts),
        scene_constant_buffer.GetAddressOf()
    );

    scene_consts.modelViewProjection = DirectX::XMMatrixIdentity() * camera.get_transform();

    update_constant_buffer(
        context.Get(),
        0,
        (void*)&scene_consts,
        sizeof(scene_consts),
        scene_constant_buffer.Get()
    );


    bind_constant_buffer(context.Get(), 0, scene_constant_buffer.Get());

    // register mouse move handler for debug mouse picking
    input::register_mouse_move_callback([this](float x, float y, WPARAM extra) -> bool  {
        static bool last_state = false;
        scene_pick(x, y);
        /*if (!last_state && input::Btn_Left(extra)) {
            last_state = true;
        } else if(last_state && !input::Btn_Left(extra)) {
            last_state = false;
        }
        */
        return true;
    });

    /*build debug line resources*/
    create_dynamic_vertex_buffer(
        device.Get(),
        debug_line_vbuf.GetAddressOf(),
        sizeof(DebugLine) * MAX_DEBUG_LINES
    );

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
        width = (float)newWidth;
        height = (float)newHeight;
        viewport.Width = width;
        viewport.Height = height;
        create_backbuffer_view();
        camera.set_aspect_ratio((float)viewport.Width/(float)viewport.Height);
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
    rasterDesc.DepthClipEnable = false;
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

// this is the one that works
void Renderer::scene_pick(float x, float y) {
    using namespace DirectX;
    XMFLOAT4X4 projection; 
    XMFLOAT4X4 view; 
    XMFLOAT4X4 inverse_view;
    XMMATRIX world;
    XMVECTOR ray_origin, ray_dir; 
    float point_x;
    float point_y;
    float point_z;

    world = XMMatrixIdentity(); // default for now is just identity matrix

    // store the camera vector matrices in addressable float matrices
    XMStoreFloat4x4(&projection, camera.get_projection());
    XMStoreFloat4x4(&view, camera.get_view());
    XMStoreFloat4x4(&inverse_view, XMMatrixInverse(nullptr, camera.get_view()));

    // transform point into view space
    point_x = (((2.0f * x) / width) - 1.0f) / projection(0, 0);
    point_y = -(((2.0f * y ) / height) - 1.0f) / projection(1, 1);
    point_z = 1.0f;

    ray_origin = XMVectorSet(0.0, 0.0, 0.0, 0.0);
    ray_dir = XMVectorSet(point_x, point_y, point_z, 1.0);

    ray_origin = XMVector3TransformCoord(ray_origin, XMMatrixInverse(nullptr, camera.get_view()));
    ray_dir = XMVector3Normalize(XMVector3TransformNormal(ray_dir, XMMatrixInverse(nullptr, camera.get_view())));

    ImGui::Text(
        "viewspace mouse: %f, %f, %f",
        point_x,
        point_y,
        point_z
    );

    print_vec("origin: ", ray_origin);
    print_vec("dir   : ", ray_dir);
    
    // find the plane equation of our rectangular quad
    XMVECTOR plane;
    XMVECTOR left_top, left_bottom, right_bottom;

    left_top = XMVectorSet( -720.0, -360.0, 0.0, 0.0);

    left_bottom = XMVectorSet( -720.0, 360.0, 0.0, 0.0);

    right_bottom = XMVectorSet( 720.0, 360.0, 0.0, 0.0);

    plane = XMPlaneFromPoints(left_top, left_bottom, right_bottom);

    print_vec("plane: ", plane);

    XMVECTOR line_begin, line_end;
    static float ray_distance = 5000.0;

    ImGui::SliderFloat("ray_distance", &ray_distance, 1.0, 5000.0);
    line_begin = ray_origin;
    line_end = XMVectorAdd(ray_origin, XMVectorScale(ray_dir, ray_distance));

    print_vec("begin: ", line_begin);
    print_vec("end: ", line_end);

    XMVECTOR intersects = XMPlaneIntersectLine(plane, line_end, line_begin);
    if(GetAsyncKeyState(VK_F1) & 1) {
        // do intersection test
        if(XMVectorGetIntX(XMVectorIsNaN(intersects)) == true) {
            add_debug_line_from_vector(line_end, line_begin,  XMFLOAT4(1.0, 0.0, 0.0, 1.0));
            //set_debug_line_from_vector(0, line_begin, line_end, XMFLOAT4(1.0, 0.0, 0.0, 1.0));
        } else {
            add_debug_line_from_vector(line_end, line_begin,  XMFLOAT4(0.0, 1.0, 0.0, 1.0));
        }
    }

    if (XMVectorGetX(intersects) > -720.0 && XMVectorGetX(intersects) < 720.0 && 
            XMVectorGetY(intersects) > -360.0 && XMVectorGetY(intersects) < 360.0) {
        set_debug_line_from_vector(0, line_begin, line_end, XMFLOAT4(0.0, 1.0, 0.2, 1.0));
    } else {
        set_debug_line_from_vector(0, line_begin, line_end, XMFLOAT4(1.0, 0.0, 0.0, 1.0));
    }
    print_vec("intersection: ", intersects);

    if(GetAsyncKeyState(VK_F2) & 1) {
        clear_debug_lines();
    }
}

void Renderer::draw_debug_lines() {
    update_dynamic_vertex_buffer(
        context.Get(),
        debug_line_vbuf.Get(),
        debug_lines,
        sizeof(DebugLine) * debug_line_count
    );

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    bind_dynamic_vertex_buffers(
        context.Get(), 
        debug_line_vbuf.GetAddressOf(), 
        sizeof(VertexPosColorTexcoord), 
        0
    );

    context->Draw(2 * debug_line_count, 0);
}

void Renderer::add_debug_line(XMFLOAT3 begin, XMFLOAT3 end, XMFLOAT4 color) {
    if(debug_line_count < MAX_DEBUG_LINES) {
        debug_lines[debug_line_count] = DebugLine{ 
            VertexPosColorTexcoord{begin, color, {0.f, 0.f}},
            VertexPosColorTexcoord{end, color, {0.f, 0.f}},
        };
        debug_line_count++;
    }
}

void Renderer::add_debug_line_from_vector(XMVECTOR begin, XMVECTOR end, XMFLOAT4 color) {
    XMFLOAT3 scalar_begin, scalar_end;
    if (debug_line_count < MAX_DEBUG_LINES) {
        XMStoreFloat3(&scalar_begin, begin);
        XMStoreFloat3(&scalar_end, end);
        add_debug_line(scalar_begin, scalar_end, color);
    }
}

void Renderer::set_debug_line(unsigned int i, XMFLOAT3 begin, XMFLOAT3 end, XMFLOAT4 color) {
    if(i < MAX_DEBUG_LINES) {
        debug_lines[i] = DebugLine{ 
            VertexPosColorTexcoord{begin, color, {0.f, 0.f}},
            VertexPosColorTexcoord{end, color, {0.f, 0.f}},
        };
    }
}

void Renderer::set_debug_line_from_vector(unsigned int i, XMVECTOR begin, XMVECTOR end, XMFLOAT4 color) {
    XMFLOAT3 scalar_begin, scalar_end;
    XMStoreFloat3(&scalar_begin, begin);
    XMStoreFloat3(&scalar_end, end);
    set_debug_line(i, scalar_begin, scalar_end, color);

}

void Renderer::clear_debug_lines() {
    for(int i = 0; i < debug_line_count; ++i) {
        debug_lines[debug_line_count] = DebugLine{ };
    }
    debug_line_count = 0;
}
