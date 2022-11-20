#include "./renderer.h"
#include "files.h"
#include "gpu_resources.h"
#include "logging.h"
#include "ray_cast.h"
#include "shader_util.h"
#include "win32_input.h"
#include <Tracy.hpp>
#include <dxgiformat.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

VertexLayout<3> VertexPosColorTexcoord::layout() {

  static VertexLayout<3> il;
  il.input_layout[0] = {"POSITION",
                        0,
                        DXGI_FORMAT_R32G32B32_FLOAT,
                        0,
                        0,
                        D3D11_INPUT_PER_VERTEX_DATA,
                        0};
  il.input_layout[1] = {"COLOR",
                        0,
                        DXGI_FORMAT_R32G32B32A32_FLOAT,
                        0,
                        12,
                        D3D11_INPUT_PER_VERTEX_DATA,
                        0};
  il.input_layout[2] = {"TEXCOORD",
                        0,
                        DXGI_FORMAT_R32G32_FLOAT,
                        0,
                        28,
                        D3D11_INPUT_PER_VERTEX_DATA,
                        0};
  return il;
}

VertexLayout<2> VertexPosColor::layout() {

  static VertexLayout<2> il;
  il.input_layout[0] = {"POSITION",
                        0,
                        DXGI_FORMAT_R32G32B32_FLOAT,
                        0,
                        0,
                        D3D11_INPUT_PER_VERTEX_DATA,
                        0};
  il.input_layout[1] = {"COLOR",
                        0,
                        DXGI_FORMAT_R32G32B32A32_FLOAT,
                        0,
                        12,
                        D3D11_INPUT_PER_VERTEX_DATA,
                        0};
  return il;
}
/*
 * Helper functions for debugging
 */

void print_vec(char *label, XMVECTOR v) {
  char buff[1024]{};
  strcat(buff, label);
  strcat(buff, "%f %f %f %f");
  ImGui::Text(buff,
              XMVectorGetX(v),
              XMVectorGetY(v),
              XMVectorGetZ(v),
              XMVectorGetW(v));
}

void Renderer::init(HWND window, u32 width, u32 height) {

  this->hwnd = window;
  this->width = width;
  this->height = height;
  this->clear_color = {0.1f, 0.1f, 0.1f, 1.0f};

  init_gfx();
  init_imgui();
  create_constant_buffer(device.Get(),
                         sizeof(PerSceneConsts),
                         scene_constant_buffer.GetAddressOf());

  scene_consts.modelViewProjection =
      DirectX::XMMatrixIdentity() * camera.get_transform();

  update_constant_buffer(context.Get(),
                         0,
                         (void *)&scene_consts,
                         sizeof(scene_consts),
                         scene_constant_buffer.Get());

  bind_constant_buffer(context.Get(), 0, scene_constant_buffer.Get());
}

Renderer::Renderer()
    : camera(DirectX::XM_PIDIV2, 16.0f / 9.0f, 10.0f, 2000.0f) {
  /* DO DEBUG NAMES */
  // static const char selection_vbuf_name[] = "Selection Vertices";
  // static const char selection_index_name[] = "Selection Indices";
  // buff_r = selection_vertex_buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
  // sizeof(selection_vbuf_name) - 1, selection_vbuf_name); buff_r =
  // selection_index_buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
  // sizeof(selection_index_name) - 1, selection_index_name);
}

Renderer::~Renderer() = default;

void Renderer::create_backbuffer_view() {
  ID3D11Texture2D *pBackBuffer;
  swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  device->CreateRenderTargetView(pBackBuffer, nullptr, &rtv);
  pBackBuffer->Release();

  D3D11_TEXTURE2D_DESC d;
  d.Width = (UINT)width;
  d.Height = (UINT)height;
  d.MipLevels = 1;
  d.ArraySize = 1;
  d.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  d.SampleDesc.Count = 1;
  d.SampleDesc.Quality = 0;
  d.Usage = D3D11_USAGE_DEFAULT;
  d.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  d.CPUAccessFlags = 0;
  d.MiscFlags = 0;

  device->CreateTexture2D(&d, NULL, depth_stencil_texture.GetAddressOf());
  device->CreateDepthStencilView(depth_stencil_texture.Get(),
                                 NULL,
                                 depth_stencil_view.GetAddressOf());
  LOG_INFO("created backbuffers");
}

void Renderer::reset_backbuffer_views() {
  depth_stencil_view.Reset();
  rtv.Reset();
}

void Renderer::resize_swapchain_backbuffer(i32 new_width,
                                           i32 new_height,
                                           b8 minimized) {
  LOG_INFO("resizing swapchain old: [{}, {}] -> [{}, {}]",
           width,
           height,
           new_width,
           new_height);
  if (device && !minimized) {
    reset_backbuffer_views();
    swapChain->ResizeBuffers(0,
                             new_width,
                             new_height,
                             DXGI_FORMAT_B8G8R8A8_UNORM,
                             0);
    width = (float)new_width;
    height = (float)new_height;
    viewport.Width = width;
    viewport.Height = height;
    create_backbuffer_view();
    camera.set_aspect_ratio((float)viewport.Width / (float)viewport.Height);
  }
}

bool Renderer::init_gfx() {
  UINT createDeviceFlags =
      D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_12_0,
                                             D3D_FEATURE_LEVEL_11_1,
                                             D3D_FEATURE_LEVEL_11_0};
  ComPtr<ID3D11Device> baseDevice;
  ComPtr<ID3D11DeviceContext> baseCtx;

  HRESULT res = D3D11CreateDevice(nullptr,
                                  D3D_DRIVER_TYPE_HARDWARE,
                                  nullptr,
                                  createDeviceFlags,
                                  featureLevels,
                                  _countof(featureLevels),
                                  D3D11_SDK_VERSION,
                                  &baseDevice,
                                  &featureLevel,
                                  &baseCtx);
  Q_ASSERT(SUCCEEDED(res));

  if (SUCCEEDED(baseDevice.As(&device))) {
    baseCtx.As(&context); // upgrade context to revision 4
  }
  {
    ComPtr<IDXGIDevice4> dxgiDevice;
    ComPtr<IDXGIFactory> dxgiBaseFact;

    if (SUCCEEDED(device.As(&dxgiDevice))) {
      ComPtr<IDXGIAdapter> adapter;
      if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter))) {
        HRESULT hr = adapter->GetParent(IID_PPV_ARGS(&dxgiBaseFact));
        if (SUCCEEDED(hr)) {
          dxgiBaseFact.As(
              &dxgiFactory); // get the factory that created the d3d11 device
        }
      }
    }
  }

  LOG_INFO("creating swapchain");
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
  dxgiFactory->CreateSwapChainForHwnd((IUnknown *)device.Get(),
                                      hwnd,
                                      &sd,
                                      nullptr,
                                      nullptr,
                                      &baseSwapChain);
  baseSwapChain.As(&swapChain); // upgrade the swapchain to revision 4

  LOG_INFO("creating blend states");
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
  LOG_INFO("creating blend states");
  D3D11_RASTERIZER_DESC2 rasterDesc{};
  rasterDesc.AntialiasedLineEnable = true;
  rasterDesc.CullMode = D3D11_CULL_BACK;
  rasterDesc.DepthBias = 0;
  rasterDesc.DepthBiasClamp = 0.0f;
  rasterDesc.DepthClipEnable =
      true; // TODO setup state for debug lines that has this as false?
  rasterDesc.FillMode = D3D11_FILL_SOLID;
  rasterDesc.FrontCounterClockwise = false;
  rasterDesc.MultisampleEnable = true;
  rasterDesc.ScissorEnable = false; // TODO true?
  rasterDesc.SlopeScaledDepthBias = 0.0f;
  res = device->CreateRasterizerState2(&rasterDesc,
                                       rasterizerState.GetAddressOf());
  Q_ASSERT(SUCCEEDED(res));
  context->RSSetState(rasterizerState.Get());

  viewport.Height = (float)height;
  viewport.Width = (float)width;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  viewport.TopLeftX = 0.0f;
  viewport.TopLeftY = 0.0;
  context->RSSetViewports(1, &viewport);

  LOG_INFO("viewport config: {} {}", height, width);

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

void Renderer::set_and_clear_backbuffer() {
  ZoneScoped("clearbb");
  context->RSSetViewports(1, &viewport);
  context->OMSetRenderTargets(1, rtv.GetAddressOf(), depth_stencil_view.Get());
  context->ClearDepthStencilView(depth_stencil_view.Get(),
                                 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                 1.0f,
                                 0);
  context->ClearRenderTargetView(rtv.Get(), (float *)&clear_color);
}

void Renderer::present() {
  ZoneScoped("present");
  swapChain->Present(1, 0);
}

bool Renderer::init_imgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  LOG_INFO("imgui version: {} docking?: {}",
           ImGui::GetVersion(),
           (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0);
  // auto font_consolas =
  // ImGui::GetIO().Fonts->AddFontFromFileTTF("c:\\windows\\fonts\\consola.ttf", 16.0f);
  ImGui_ImplDX11_Init(device.Get(), context.Get());
  ImGui_ImplWin32_Init(hwnd);
  return true;
}

void Renderer::imgui_frame_begin() {
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
}

void Renderer::imgui_frame_end() {
  ZoneScoped("imgui_end");
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

// void Renderer::add_selection_rect(float left, float right, float top, float
// bottom) {
//     if (selection_count < MaxSelections) {
//         selections[selection_count] = SelectionArea{left, right, top,
//         bottom}; selection_count++;
//     }
// }
//
// void Renderer::set_selection_rect(int index, float left, float right, float
// top, float bottom) {
//     if (index < MaxSelections) {
//         selections[index] = SelectionArea{left, right, top, bottom};
//     }
// }
//
// void Renderer::draw_selection_rect() {
//
//     /*
//      * build selection drawing resources
//      * 1 quad = 4 vertices
//      * 1 quad = 6 indices
//      * 4 quads for Border + 1 quad for internal
//      * num_verts = MaxSelections * (4 + 1)      * 4
//      *                              ^num quads  ^num verts per quad
//      * num_indices = MaxSelections * ( 4 + 1 ) * 6
//      * this can be optimzied because there are shared vertices on the
//      * edges of the rectangle
//      *
//      *
//        tesellate the selection rects
//
//        +------------+
//        |            |
//        |            |
//        |            |
//        |            |
//        +------------+
//     */
//     //
//     constexpr auto VertexStride = 4 * 5; // there are 5 quads, 4 verts each
//     constexpr auto IndexStride  = 6 * 5; // there are 5 quads, 6 indices each
//     float t = selection_border_thickness;
//
//     // using vector here because a stack array would overflow the stack, we
//     must use heap mem std::vector<VertexPosColorTexcoord> selection_vertices;
//     std::vector<int> indices;
//
//     selection_vertices.resize(VertexStride * MaxSelections); // each
//     selection is 5 quads, there are 4 verts per quad
//     indices.resize(IndexStride * MaxSelections);
//
//     for(int i = 0; i < selection_count; ++i) {
//         auto q = selections[i];
//         // the main quad
//         VertexPosColorTexcoord quad_verts[] = {
//             {
//                 DirectX::XMFLOAT3{q.left, q.top, -1.0f},
//                 selection_inner_color,
//                 DirectX::XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 DirectX::XMFLOAT3{q.right, q.top, -1.0f},
//                 selection_inner_color,
//                 DirectX::XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 DirectX::XMFLOAT3{q.left, q.bottom, -1.0f},
//                 selection_inner_color,
//                 DirectX::XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 DirectX::XMFLOAT3{q.right, q.bottom, -1.0f},
//                 selection_inner_color,
//                 DirectX::XMFLOAT2{0.0f, 0.0f}
//             },
//         };
//
//         VertexPosColorTexcoord top_border[] = {
//             {
//                 XMFLOAT3{q.left, q.top - t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.right, q.top - t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.left, q.top, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.right, q.top, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//         };
//
//         VertexPosColorTexcoord left_border[] = {
//             {
//                 XMFLOAT3{q.left- t , q.top - t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.left, q.top - t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.left - t, q.bottom + t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.left, q.bottom + t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//         };
//
//         VertexPosColorTexcoord right_border[] = {
//             {
//                 XMFLOAT3{q.right, q.top - t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.right + t, q.top - t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.right, q.bottom + t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.right + t, q.bottom + t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//         };
//         VertexPosColorTexcoord bottom_border[] = {
//             {
//                 XMFLOAT3{q.left, q.bottom, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.right, q.bottom, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.left, q.bottom + t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//             {
//                 XMFLOAT3{q.right, q.bottom + t, -1.0f},
//                 selection_border_color,
//                 XMFLOAT2{0.0f, 0.0f}
//             },
//         };
//
//         selection_vertices[i * VertexStride + 0] = quad_verts[0];
//         selection_vertices[i * VertexStride + 1] = quad_verts[1];
//         selection_vertices[i * VertexStride + 2] = quad_verts[2];
//         selection_vertices[i * VertexStride + 3] = quad_verts[3];
//
//         selection_vertices[i * VertexStride + 4] = top_border[0];
//         selection_vertices[i * VertexStride + 5] = top_border[1];
//         selection_vertices[i * VertexStride + 6] = top_border[2];
//         selection_vertices[i * VertexStride + 7] = top_border[3];
//
//         selection_vertices[i * VertexStride + 8]  =  left_border[0];
//         selection_vertices[i * VertexStride + 9]  =  left_border[1];
//         selection_vertices[i * VertexStride + 10] = left_border[2];
//         selection_vertices[i * VertexStride + 11] = left_border[3];
//
//         selection_vertices[i * VertexStride + 12] =  bottom_border[0];
//         selection_vertices[i * VertexStride + 13] =  bottom_border[1];
//         selection_vertices[i * VertexStride + 14] = bottom_border[2];
//         selection_vertices[i * VertexStride + 15] = bottom_border[3];
//
//         selection_vertices[i * VertexStride + 16] = right_border[0];
//         selection_vertices[i * VertexStride + 17] = right_border[1];
//         selection_vertices[i * VertexStride + 18] = right_border[2];
//         selection_vertices[i * VertexStride + 19] = right_border[3];
//
//         indices[i * IndexStride + 0] =  i   * VertexStride + 2;
//         indices[i * IndexStride + 1] =  i   * VertexStride + 3;
//         indices[i * IndexStride + 2] =  i   * VertexStride + 1;
//         indices[i * IndexStride + 3] =  i   * VertexStride + 2;
//         indices[i * IndexStride + 4] =  i   * VertexStride + 1;
//         indices[i * IndexStride + 5] =  i   * VertexStride + 0;
//
//         indices[i * IndexStride + 6] =  i   * VertexStride + 2 + 4;
//         indices[i * IndexStride + 7] =  i   * VertexStride + 3 + 4;
//         indices[i * IndexStride + 8] =  i   * VertexStride + 1 + 4;
//         indices[i * IndexStride + 9] =  i   * VertexStride + 2 + 4;
//         indices[i * IndexStride + 10] = i   * VertexStride + 1 + 4;
//         indices[i * IndexStride + 11] = i   * VertexStride + 0 + 4;
//
//         indices[i * IndexStride + 12] = i   * VertexStride + 2 + 8;
//         indices[i * IndexStride + 13] = i   * VertexStride + 3 + 8;
//         indices[i * IndexStride + 14] = i   * VertexStride + 1 + 8;
//         indices[i * IndexStride + 15] = i   * VertexStride + 2 + 8;
//         indices[i * IndexStride + 16] = i   * VertexStride + 1 + 8;
//         indices[i * IndexStride + 17] = i   * VertexStride + 0 + 8;
//
//         indices[i * IndexStride + 18] = i   * VertexStride + 2 + 12;
//         indices[i * IndexStride + 19] = i   * VertexStride + 3 + 12;
//         indices[i * IndexStride + 20] = i   * VertexStride + 1 + 12;
//         indices[i * IndexStride + 21] = i   * VertexStride + 2 + 12;
//         indices[i * IndexStride + 22] = i   * VertexStride + 1 + 12;
//         indices[i * IndexStride + 23] = i   * VertexStride + 0 + 12;
//
//         indices[i * IndexStride + 24] = i   * VertexStride + 2 + 16;
//         indices[i * IndexStride + 25] = i   * VertexStride + 3 + 16;
//         indices[i * IndexStride + 26] = i   * VertexStride + 1 + 16;
//         indices[i * IndexStride + 27] = i   * VertexStride + 2 + 16;
//         indices[i * IndexStride + 28] = i   * VertexStride + 1 + 16;
//         indices[i * IndexStride + 29] = i   * VertexStride + 0 + 16;
//     }
//
//     update_dynamic_vertex_buffer(
//         context.Get(),
//         selection_vertex_buffer.Get(),
//         (void*)selection_vertices.data(),
//         sizeof(VertexPosColorTexcoord) * selection_vertices.size()
//     );
//
//     update_dynamic_index_buffer(
//         context.Get(),
//         selection_index_buffer.Get(),
//         indices.data(),
//         indices.size()
//     );
//
//     bind_dynamic_vertex_buffers(
//         context.Get(),
//         selection_vertex_buffer.GetAddressOf(),
//         sizeof(VertexPosColorTexcoord),
//         0
//     );
//
//     bind_dynamic_index_buffer(context.Get(), selection_index_buffer.Get());
//
//     context->DrawIndexed(indices.size(), 0, 0);
// }

// use debug lines array for now
void Renderer::create_world_grid() {
  XMFLOAT4 line_color{0.7f, 0.7f, 0.7f, 1.0f};
  static bool draw = true;
  static float grid_size = 64.0f;
  static int max_lines = 500; // 250 horizontal and 250 vertical (y axis)
  static float span = (float)(grid_size * max_lines);

  for (int i = -(max_lines / 2); i < max_lines / 2; ++i) {
    /*
    add_debug_line_from_vector(XMVectorSet(grid_size * (float)i, -span, 0.0f,
    0.0f), XMVectorSet(grid_size * (float)i, span, 0.0f, 0.0f), line_color);
    add_debug_line_from_vector(XMVectorSet( -span, grid_size * (float)i, 0.0f,
    0.0f), XMVectorSet(span, grid_size*(float)i,  0.0f, 0.0f), line_color);
    */
  }
}

// use debug lines array for now
void Renderer::create_world_grid_horizon() {
  XMFLOAT4 line_color{0.7f, 0.7f, 0.7f, 1.0f};
  static bool draw = true;
  static float grid_size = 64.0f;
  static int max_lines = 500; // 250 horizontal and 250 vertical (y axis)
  static float span = (float)(grid_size * max_lines);

  for (int i = -(max_lines / 2); i < max_lines / 2; ++i) {
    /*add_debug_line_from_vector(
        XMVectorSet(grid_size * (float)i, 0, -span, 0.0),
        XMVectorSet(grid_size * (float)i, 0, span, 0),
        line_color
    );

    add_debug_line_from_vector(
        XMVectorSet( -span, 0.0f, grid_size * (float)i, 0),
        XMVectorSet(span, 0.0f, grid_size*(float)i, 0.0f),
        line_color
    );*/
  }
}

void Renderer::create_vertex_buffer(ID3D11Buffer **out_buffer, u32 size_bytes) {
  D3D11_BUFFER_DESC bd{};
  bd.ByteWidth = size_bytes;
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;
  auto hres = device->CreateBuffer(&bd, nullptr, out_buffer);
  Q_ASSERT(SUCCEEDED(hres));
}

void Renderer::create_index_buffer(ID3D11Buffer **out_buffer, u32 num_indices) {
  D3D11_BUFFER_DESC bd{};
  bd.ByteWidth = sizeof(u32) * num_indices;
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;
  auto hres = device->CreateBuffer(&bd, nullptr, out_buffer);
  Q_ASSERT(SUCCEEDED(hres));
}

void Renderer::create_pixel_shader(fs::path src_path,
                                   ID3D11PixelShader **out_shader) {
  ComPtr<ID3DBlob> psb;

  bool r = shader_compile_disk(src_path, "ps_main", "ps_5_0", &psb);
  Q_ASSERT(r);
  HRESULT hr = device->CreatePixelShader(psb->GetBufferPointer(),
                                         psb->GetBufferSize(),
                                         nullptr,
                                         out_shader);
  Q_ASSERT(SUCCEEDED(hr));
}

void Renderer::set_vertex_buffer(ID3D11Buffer **buffers,
                                 u32 buff_count,
                                 u32 stride,
                                 u32 offset) {
  context->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
}

void Renderer::set_index_buffer(ID3D11Buffer *buffer) {
  context->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, 0);
}

void Renderer::set_pixel_shader(ID3D11PixelShader *shader) {
  context->PSSetShader(shader, nullptr, 0);
}

void Renderer::set_vertex_shader(ID3D11VertexShader *shader) {
  context->VSSetShader(shader, nullptr, 0);
}

void Renderer::draw_indexed(u32 num_indices) {
  context->DrawIndexed(num_indices, 0, 0);
}

void Renderer::draw(u32 vert_count) { context->Draw(vert_count, 0); }
void Renderer::set_input_layout(ID3D11InputLayout *il) {
  context->IASetInputLayout(il);
}

void Renderer::set_topology(D3D11_PRIMITIVE_TOPOLOGY topo) {
  context->IASetPrimitiveTopology(topo);
}

f32 Renderer::get_aspect_ratio() { return width / height; }
