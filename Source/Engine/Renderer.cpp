#include "./renderer.h"
#include "gpu_resources.h"
#include "shader_util.h"
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <Tracy.hpp>
#include "win32_input.h"
#include "files.h"
#include "ray_cast.h"

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
    clearColor = { 0.1f, 0.1f, 0.1f, 1.0f}; 

    /*create scene resources*/
    HRESULT buff_r = create_dynamic_vertex_buffer(device.Get(), 
        scene_vertex_buffer.GetAddressOf(), 
        sizeof(VertexPosColorTexcoord) * 10000
    );

    if(FAILED(buff_r)) {
        // debug
        __debugbreak();
    }

    buff_r = create_dynamic_index_buffer(
        device.Get(), 
        scene_index_buffer.GetAddressOf(), 
        10000 * 6
    );

    if(FAILED(buff_r)) { 
        __debugbreak();
    }


    auto default_shader_path = files::get_shader_root() / "TexturedQuad.hlsl";
    build_vertex_shader(
        device.Get(), 
        default_shader_path.wstring().c_str(), 
        scene_vertex_shader.GetAddressOf(), 
        vtx_pos_color_tex_il.GetAddressOf()
    );

    build_pixel_shader(
        device.Get(), 
        default_shader_path.wstring().c_str(), 
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


    /*build debug line resources*/
    create_dynamic_vertex_buffer(
        device.Get(),
        debug_line_vbuf.GetAddressOf(),
        sizeof(DebugLine) * MAX_DEBUG_LINES
    );


    /*
     * build selection drawing resources
     * 1 quad = 4 vertices
     * 1 quad = 6 indices
     * 4 quads for Border + 1 quad for internal 
     * num_verts = MaxSelections * (4 + 1)      * 4
     *                              ^num quads  ^num verts per quad
     * num_indices = MaxSelections * ( 4 + 1 ) * 6 
     * this can be optimzied because there are shared vertices on the 
     * edges of the rectangle
     * 
     * */

    buff_r = create_dynamic_vertex_buffer(device.Get(), 
        selection_vertex_buffer.GetAddressOf(), 
        SelectionsVertexSize
    );

   

    if(FAILED(buff_r)) {
        // debug
        __debugbreak();
    }

    buff_r = create_dynamic_index_buffer(
        device.Get(), 
        selection_index_buffer.GetAddressOf(), 
        SelectionsIndexSize
    );

    if(FAILED(buff_r)) {
        __debugbreak();
    }

    //Input::Ui::register_callback([this](Input::Ui::MouseState const& mouse, Input::Ui::KeyboardState const& kbd ) -> bool {
    //    scene_pick(mouse.x, mouse.y);
    //    return true;
    //});

    /* DO DEBUG NAMES */
    static const char selection_vbuf_name[] = "Selection Vertices";
    static const char selection_index_name[] = "Selection Indices";
    buff_r = selection_vertex_buffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(selection_vbuf_name) - 1, selection_vbuf_name);
    buff_r = selection_index_buffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(selection_index_name) - 1, selection_index_name);
}

Renderer::~Renderer() = default;

void Renderer::create_backbuffer_view()
{
    ID3D11Texture2D *pBackBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    device->CreateRenderTargetView(pBackBuffer, nullptr, &rtv);
    pBackBuffer->Release();

    D3D11_TEXTURE2D_DESC d;
    d.Width = (UINT)width;
    d.Height = (UINT)height;
    d.MipLevels =1;
    d.ArraySize =1 ;
    d.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    d.SampleDesc.Count = 1;
    d.SampleDesc.Quality = 0;
    d.Usage = D3D11_USAGE_DEFAULT;
    d.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    d.CPUAccessFlags = 0;
    d.MiscFlags = 0;

    device->CreateTexture2D(&d, NULL, depth_stencil_texture.GetAddressOf());
    device->CreateDepthStencilView(depth_stencil_texture.Get(), NULL, depth_stencil_view.GetAddressOf());
}

void Renderer::reset_backbuffer_views()
{
    depth_stencil_view.Reset();
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
    rasterDesc.AntialiasedLineEnable = true;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true; // TODO setup state for debug lines that has this as false?
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = true;
    rasterDesc.ScissorEnable = false; // TODO true?
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
    context->OMSetRenderTargets(1, rtv.GetAddressOf(), depth_stencil_view.Get());
    context->ClearDepthStencilView(depth_stencil_view.Get(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
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

    auto font_consolas = ImGui::GetIO().Fonts->AddFontFromFileTTF("c:\\windows\\fonts\\consola.ttf", 16.0f);
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

    ray_cast::screen_to_world_ray(x, y, width, height, camera, world, ray_origin, ray_dir);

    float left = -720.0, right = 720.0, top = -360.0, bottom = 360.0;
    bool hit = ray_cast::against_quad(ray_origin, ray_dir, left, right, top, bottom);
    if (hit) {
        set_selection_rect(0, -720, 720, -360, 360);
    } else {
        set_selection_rect(0, 0, 0, 0, 0);
    }

    return;
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

void Renderer::set_debug_line_color(unsigned int i, XMFLOAT4 color) {
    if(i < MAX_DEBUG_LINES) {
        debug_lines[i].begin.color = color;
        debug_lines[i].end.color = color;
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

void Renderer::add_selection_rect(float left, float right, float top, float bottom) {
    if (selection_count < MaxSelections) {
        selections[selection_count] = SelectionArea{left, right, top, bottom};
        selection_count++;
    }
}

void Renderer::set_selection_rect(int index, float left, float right, float top, float bottom) {
    if (index < MaxSelections) {
        selections[index] = SelectionArea{left, right, top, bottom};
    }
}

void Renderer::draw_selection_rect() {

    /*
     * build selection drawing resources
     * 1 quad = 4 vertices
     * 1 quad = 6 indices
     * 4 quads for Border + 1 quad for internal 
     * num_verts = MaxSelections * (4 + 1)      * 4
     *                              ^num quads  ^num verts per quad
     * num_indices = MaxSelections * ( 4 + 1 ) * 6 
     * this can be optimzied because there are shared vertices on the 
     * edges of the rectangle
     * 
     * 
       tesellate the selection rects
      
       +------------+
       |            |    
       |            |
       |            |
       |            |
       +------------+
    */
    // 
    constexpr auto VertexStride = 4 * 5; // there are 5 quads, 4 verts each
    constexpr auto IndexStride  = 6 * 5; // there are 5 quads, 6 indices each 
    float t = selection_border_thickness;
     
    // using vector here because a stack array would overflow the stack, we must use heap mem
    std::vector<VertexPosColorTexcoord> selection_vertices;
    std::vector<int> indices;

    selection_vertices.resize(VertexStride * MaxSelections); // each selection is 5 quads, there are 4 verts per quad
    indices.resize(IndexStride * MaxSelections);

    for(int i = 0; i < selection_count; ++i) {
        auto q = selections[i];
        // the main quad
        VertexPosColorTexcoord quad_verts[] = {
            {
                DirectX::XMFLOAT3{q.left, q.top, -1.0f},
                selection_inner_color,
                DirectX::XMFLOAT2{0.0f, 0.0f}
            },   
            {
                DirectX::XMFLOAT3{q.right, q.top, -1.0f},
                selection_inner_color,
                DirectX::XMFLOAT2{0.0f, 0.0f}
            },
            {
                DirectX::XMFLOAT3{q.left, q.bottom, -1.0f},
                selection_inner_color,
                DirectX::XMFLOAT2{0.0f, 0.0f}
            }, 
            {
                DirectX::XMFLOAT3{q.right, q.bottom, -1.0f},
                selection_inner_color,
                DirectX::XMFLOAT2{0.0f, 0.0f}
            },   
        };

        VertexPosColorTexcoord top_border[] = {
            {
                XMFLOAT3{q.left, q.top - t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },   
            {
                XMFLOAT3{q.right, q.top - t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },
            {
                XMFLOAT3{q.left, q.top, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            }, 
            {
                XMFLOAT3{q.right, q.top, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },   
        };

        VertexPosColorTexcoord left_border[] = {
            {
                XMFLOAT3{q.left- t , q.top - t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },   
            {
                XMFLOAT3{q.left, q.top - t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },
            {
                XMFLOAT3{q.left - t, q.bottom + t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            }, 
            {
                XMFLOAT3{q.left, q.bottom + t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },   
        };

        VertexPosColorTexcoord right_border[] = {
            {
                XMFLOAT3{q.right, q.top - t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },   
            {
                XMFLOAT3{q.right + t, q.top - t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },
            {
                XMFLOAT3{q.right, q.bottom + t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            }, 
            {
                XMFLOAT3{q.right + t, q.bottom + t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },   
        };
        VertexPosColorTexcoord bottom_border[] = {
            {
                XMFLOAT3{q.left, q.bottom, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },   
            {
                XMFLOAT3{q.right, q.bottom, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },
            {
                XMFLOAT3{q.left, q.bottom + t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            }, 
            {
                XMFLOAT3{q.right, q.bottom + t, -1.0f},
                selection_border_color,
                XMFLOAT2{0.0f, 0.0f}
            },   
        };

        selection_vertices[i * VertexStride + 0] = quad_verts[0];
        selection_vertices[i * VertexStride + 1] = quad_verts[1];
        selection_vertices[i * VertexStride + 2] = quad_verts[2];
        selection_vertices[i * VertexStride + 3] = quad_verts[3];

        selection_vertices[i * VertexStride + 4] = top_border[0];
        selection_vertices[i * VertexStride + 5] = top_border[1];
        selection_vertices[i * VertexStride + 6] = top_border[2];
        selection_vertices[i * VertexStride + 7] = top_border[3];

        selection_vertices[i * VertexStride + 8]  =  left_border[0];
        selection_vertices[i * VertexStride + 9]  =  left_border[1];
        selection_vertices[i * VertexStride + 10] = left_border[2];
        selection_vertices[i * VertexStride + 11] = left_border[3];

        selection_vertices[i * VertexStride + 12] =  bottom_border[0];
        selection_vertices[i * VertexStride + 13] =  bottom_border[1];
        selection_vertices[i * VertexStride + 14] = bottom_border[2];
        selection_vertices[i * VertexStride + 15] = bottom_border[3];

        selection_vertices[i * VertexStride + 16] = right_border[0];
        selection_vertices[i * VertexStride + 17] = right_border[1];
        selection_vertices[i * VertexStride + 18] = right_border[2];
        selection_vertices[i * VertexStride + 19] = right_border[3];

        indices[i * IndexStride + 0] =  i   * VertexStride + 2;
        indices[i * IndexStride + 1] =  i   * VertexStride + 3;
        indices[i * IndexStride + 2] =  i   * VertexStride + 1;
        indices[i * IndexStride + 3] =  i   * VertexStride + 2;
        indices[i * IndexStride + 4] =  i   * VertexStride + 1;
        indices[i * IndexStride + 5] =  i   * VertexStride + 0;

        indices[i * IndexStride + 6] =  i   * VertexStride + 2 + 4;
        indices[i * IndexStride + 7] =  i   * VertexStride + 3 + 4;
        indices[i * IndexStride + 8] =  i   * VertexStride + 1 + 4;
        indices[i * IndexStride + 9] =  i   * VertexStride + 2 + 4;
        indices[i * IndexStride + 10] = i   * VertexStride + 1 + 4;
        indices[i * IndexStride + 11] = i   * VertexStride + 0 + 4;

        indices[i * IndexStride + 12] = i   * VertexStride + 2 + 8;
        indices[i * IndexStride + 13] = i   * VertexStride + 3 + 8;
        indices[i * IndexStride + 14] = i   * VertexStride + 1 + 8;
        indices[i * IndexStride + 15] = i   * VertexStride + 2 + 8;
        indices[i * IndexStride + 16] = i   * VertexStride + 1 + 8;
        indices[i * IndexStride + 17] = i   * VertexStride + 0 + 8;

        indices[i * IndexStride + 18] = i   * VertexStride + 2 + 12;
        indices[i * IndexStride + 19] = i   * VertexStride + 3 + 12;
        indices[i * IndexStride + 20] = i   * VertexStride + 1 + 12;
        indices[i * IndexStride + 21] = i   * VertexStride + 2 + 12;
        indices[i * IndexStride + 22] = i   * VertexStride + 1 + 12;
        indices[i * IndexStride + 23] = i   * VertexStride + 0 + 12;

        indices[i * IndexStride + 24] = i   * VertexStride + 2 + 16;
        indices[i * IndexStride + 25] = i   * VertexStride + 3 + 16;
        indices[i * IndexStride + 26] = i   * VertexStride + 1 + 16;
        indices[i * IndexStride + 27] = i   * VertexStride + 2 + 16;
        indices[i * IndexStride + 28] = i   * VertexStride + 1 + 16;
        indices[i * IndexStride + 29] = i   * VertexStride + 0 + 16;
    }

    update_dynamic_vertex_buffer(
        context.Get(),
        selection_vertex_buffer.Get(),
        (void*)selection_vertices.data(),
        sizeof(VertexPosColorTexcoord) * selection_vertices.size()
    );

    update_dynamic_index_buffer(
        context.Get(), 
        selection_index_buffer.Get(), 
        indices.data(),
        indices.size()
    );

    bind_dynamic_vertex_buffers(
        context.Get(),
        selection_vertex_buffer.GetAddressOf(),
        sizeof(VertexPosColorTexcoord),
        0
    );

    bind_dynamic_index_buffer(context.Get(), selection_index_buffer.Get());

    context->DrawIndexed(indices.size(), 0, 0);
}

void Renderer::imgui_draw_screen_rect(float left, float right, float top, float bottom) {
    auto draw_list = ImGui::GetForegroundDrawList();
    // imgui does BRG 
    draw_list->AddRectFilled(ImVec2(left, top), ImVec2(right, bottom), ImU32(0x70FFBE00));
    draw_list->AddRect(ImVec2(left, top), ImVec2(right, bottom), ImU32(0xFFFFBE00));
}

// use debug lines array for now
void Renderer::create_world_grid() {
    XMFLOAT4 line_color{0.7f, 0.7f, 0.7f, 1.0f};
    static bool draw = true;
    static float grid_size = 64.0f;
    static int max_lines = 500;// 250 horizontal and 250 vertical (y axis)
    static float span = (float)(grid_size * max_lines); 


    for(int i = -(max_lines/2); i < max_lines/2; ++i) {
        add_debug_line_from_vector(XMVectorSet(grid_size * (float)i, -span, 0.0f, 0.0f), XMVectorSet(grid_size * (float)i, span, 0.0f, 0.0f), line_color);
        add_debug_line_from_vector(XMVectorSet( -span, grid_size * (float)i, 0.0f, 0.0f), XMVectorSet(span, grid_size*(float)i,  0.0f, 0.0f), line_color);
    }

}
