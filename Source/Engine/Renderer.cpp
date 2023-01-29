#include "./renderer.h"
#include "files.h"
#include "gpu_resources.h"
#include "logging.h"
#include "ray_cast.h"
#include "shader_util.h"
#include <Tracy.hpp>
#include <d3d11.h>
#include <d3dcommon.h>
#include <dxgiformat.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include "win32_lib.h"

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

Renderer::Renderer(HWND hwnd, i32 width, i32 height) {
  this->width = width;
  this->height = height;
  this->hwnd = hwnd;
  d3d11_init(hwnd, width, height);

  /* DO DEBUG NAMES */
  // static const char selection_vbuf_name[] = "Selection Vertices";
  // static const char selection_index_name[] = "Selection Indices";
  // buff_r = selection_vertex_buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
  // sizeof(selection_vbuf_name) - 1, selection_vbuf_name); buff_r =
  // selection_index_buffer->SetPrivateData(WKPDID_D3DDebugObjectName,
  // sizeof(selection_index_name) - 1, selection_index_name);
}

Renderer::~Renderer() = default;

bool Renderer::d3d11_init(HWND hwnd, i32 width, i32 height) {
  UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
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
          dxgiBaseFact.As(&dxgiFactory); // get the factory that created the d3d11 device
        }
      }
    }
  }

  LOG_INFO("creating swapchain");
  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC1 sd = {0};
  sd.Width = (u32)width;
  sd.Height = (u32)height;
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
  alphaBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
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
  rasterDesc.DepthClipEnable = true; // TODO setup state for debug lines that has this as false?
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
  gridSampler.BorderColor[0] = 0.f;
  gridSampler.BorderColor[1] = 0.f;
  gridSampler.BorderColor[2] = 0.f;
  gridSampler.BorderColor[3] = 0.f;
  gridSampler.MinLOD = -FLT_MAX;
  gridSampler.MaxLOD = FLT_MAX;
  device->CreateSamplerState(&gridSampler, &gridSS);
  context->PSSetSamplers(0, 1, gridSS.GetAddressOf());

  backbuffer_view_create();

  LOG_INFO("Creating scene render texture");

  render_texture_create();

  LOG_INFO("creating fullscreen quad shaders");

  auto fs_shader = Files::get_shader_root() / "fullscreen_quad.hlsl";
  this->pixel_shader_create(fs_shader, &ps_fullscreen_quad);
  this->vertex_shader_create(fs_shader, 
      VertexPosColor::layout(),
      &vs_fullscreen_quad,
      &fullscreen_quad_il);

  return true;
}

void Renderer::present() {
  ZoneScoped("present");
  swapChain->Present(1, 0);
}




f32 Renderer::backbuffer_aspect_ratio() { return width / height; }

void Renderer::backbuffer_view_create() {
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

void Renderer::backbuffer_view_reset() {
  depth_stencil_view.Reset();
  rtv.Reset();
}

void Renderer::backbuffer_resize(i32 new_width,
                                           i32 new_height,
                                           b8 minimized) {
  LOG_INFO("resizing swapchain old: [{}, {}] -> [{}, {}]",
           width,
           height,
           new_width,
           new_height);
  if (device && !minimized) {
    backbuffer_view_reset();
    swapChain->ResizeBuffers(0,
                             new_width,
                             new_height,
                             DXGI_FORMAT_B8G8R8A8_UNORM,
                             0);
    width = (float)new_width;
    height = (float)new_height;
    viewport.Width = width;
    viewport.Height = height;
    backbuffer_view_create();
  }
}

void Renderer::backbuffer_clear(Float4 clear_color) {
  //context->RSSetViewports(1, &viewport);
  //context->OMSetRenderTargets(1, rtv.GetAddressOf(), depth_stencil_view.Get());
 // context->ClearDepthStencilView(depth_stencil_view.Get(),
 //                                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
 //                                1.0f,
 //                                0);

  context->ClearRenderTargetView(rtv.Get(), (float*)&clear_color);
}


void Renderer::vertex_buffer_create(ID3D11Buffer **out_buffer, u32 size_bytes) {
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

void Renderer::vertex_buffer_bind(ID3D11Buffer **buffers,
                                 u32 buff_count,
                                 u32 stride,
                                 u32 offset) {
  context->IASetVertexBuffers(0, 1, buffers, &stride, &offset);
}

void Renderer::index_buffer_create(ID3D11Buffer **out_buffer, u32 num_indices) {
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

void Renderer::pixel_shader_create(fs::path src_path,
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

void Renderer::pixel_shader_bind(ID3D11PixelShader *shader) {
  context->PSSetShader(shader, nullptr, 0);
}


void Renderer::input_layout_bind(ID3D11InputLayout *il) {
  context->IASetInputLayout(il);
}

void Renderer::index_buffer_bind(ID3D11Buffer *buffer) {
  context->IASetIndexBuffer(buffer, DXGI_FORMAT_R32_UINT, 0);
}


void Renderer::vertex_shader_bind(ID3D11VertexShader *shader) {
  context->VSSetShader(shader, nullptr, 0);
}

void Renderer::draw_indexed(u32 num_indices) {
  context->DrawIndexed(num_indices, 0, 0);
}

void Renderer::draw(u32 vert_count) { context->Draw(vert_count, 0); }


void Renderer::set_topology(D3D11_PRIMITIVE_TOPOLOGY topo) {
  context->IASetPrimitiveTopology(topo);
}



void Renderer::render_texture_clear(Float4 clear_color) {
  context->OMSetRenderTargets(1,
                              render_texture.render_target_view.GetAddressOf(),
                              nullptr);
  // dont know if we use the depth here...?

  context->ClearRenderTargetView(render_texture.render_target_view.Get(),
                                 (float *)&clear_color);

  float blend_factor[] = {0, 0, 0, 0};
  context->OMSetBlendState(alphaBlendState.Get(), blend_factor, 0xffffffff);
}

void Renderer::render_texture_resize(float w, float h) {
  D3D11_TEXTURE2D_DESC texture_desc{};
  D3D11_RENDER_TARGET_VIEW_DESC view_desc{};
  D3D11_SHADER_RESOURCE_VIEW_DESC shader_view_desc{};

  texture_desc.Width = w;
  texture_desc.Height = h;
  texture_desc.MipLevels = 1;
  texture_desc.ArraySize = 1;
  texture_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  texture_desc.SampleDesc.Count = 1;
  texture_desc.Usage = D3D11_USAGE_DEFAULT;
  texture_desc.BindFlags =
      D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  texture_desc.CPUAccessFlags = 0;
  texture_desc.MiscFlags = 0;

  HRESULT r = device->CreateTexture2D(&texture_desc,
                                      nullptr,
                                      &render_texture.render_target);
  LOG_COM(r);
  view_desc.Format = texture_desc.Format;
  view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  view_desc.Texture2D.MipSlice = 0;
  r = device->CreateRenderTargetView(render_texture.render_target.Get(),
                                     &view_desc,
                                     &render_texture.render_target_view);
  LOG_COM(r);
  shader_view_desc.Format = texture_desc.Format;
  shader_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shader_view_desc.Texture2D.MostDetailedMip = 0;
  shader_view_desc.Texture2D.MipLevels = 1;
  r = device->CreateShaderResourceView(render_texture.render_target.Get(),
                                       &shader_view_desc,
                                       &render_texture.srv);
  LOG_COM(r);
}

void Renderer::render_texture_create() {
  D3D11_TEXTURE2D_DESC texture_desc{};
  D3D11_RENDER_TARGET_VIEW_DESC view_desc{};
  D3D11_SHADER_RESOURCE_VIEW_DESC shader_view_desc{};

  texture_desc.Width = width / 2;
  texture_desc.Height = height / 2;
  texture_desc.MipLevels = 1;
  texture_desc.ArraySize = 1;
  texture_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  texture_desc.SampleDesc.Count = 1;
  texture_desc.Usage = D3D11_USAGE_DEFAULT;
  texture_desc.BindFlags =
      D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  texture_desc.CPUAccessFlags = 0;
  texture_desc.MiscFlags = 0;

  HRESULT r = device->CreateTexture2D(&texture_desc,
                                      nullptr,
                                      &render_texture.render_target);
  LOG_COM(r);
  view_desc.Format = texture_desc.Format;
  view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  view_desc.Texture2D.MipSlice = 0;
  r = device->CreateRenderTargetView(render_texture.render_target.Get(),
                                     &view_desc,
                                     &render_texture.render_target_view);
  LOG_COM(r);
  shader_view_desc.Format = texture_desc.Format;
  shader_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shader_view_desc.Texture2D.MostDetailedMip = 0;
  shader_view_desc.Texture2D.MipLevels = 1;
  r = device->CreateShaderResourceView(render_texture.render_target.Get(),
                                       &shader_view_desc,
                                       &render_texture.srv);
  LOG_COM(r);
}

void Renderer::set_viewport(ViewportRegion viewport) {
  D3D11_VIEWPORT vp{};
  vp.TopLeftX = viewport.x;
  vp.TopLeftY = viewport.y;
  vp.MinDepth = 0.0;
  vp.MaxDepth = 1.0;
  vp.Width = viewport.w;
  vp.Height = viewport.h;
  context->RSSetViewports(1, &vp);
}


bool Renderer::texture_create(TextureParams const &params,
                              Texture2D &out_texture) {
  HRESULT check;
  D3D11_TEXTURE2D_DESC tex_desc{};
  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
  D3D11_SUBRESOURCE_DATA sr{};
  ID3D11Texture2D *texture{};

  // make sure requested width and height are acceptable
  if (!(params.desired_width < D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION &&
        params.desired_height < D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION)) {
    LOG_WARNING("requested texture size is too big: w= {} h= {}",
                params.desired_width,
                params.desired_height);
    return false;
  }

  out_texture.width = params.desired_width;
  out_texture.height = params.desired_height;

  // create the texture
  tex_desc.Width = params.desired_width;
  tex_desc.Height = params.desired_height;
  tex_desc.MipLevels = 1;
  tex_desc.ArraySize = 1;
  tex_desc.Format = (DXGI_FORMAT)params.format;
  tex_desc.SampleDesc.Count = 1;
  tex_desc.Usage = params.usage;
  tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  tex_desc.CPUAccessFlags = params.cpu_flags;

  sr.pSysMem = params.initial_data;
  sr.SysMemPitch = tex_desc.Width * 4;
  sr.SysMemSlicePitch = 0;

  if (params.initial_data == nullptr) {
    check = device->CreateTexture2D(&tex_desc, nullptr, &out_texture.texture);
  } else {
    check = device->CreateTexture2D(&tex_desc, &sr, &out_texture.texture);
  }

  if (!SUCCEEDED(check)) {
    LOG_COM(check);
    return false;
  }

  srv_desc.Format = (DXGI_FORMAT)params.format;
  srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = tex_desc.MipLevels;
  srv_desc.Texture2D.MostDetailedMip = 0;
  check = device->CreateShaderResourceView(out_texture.texture.Get(),
                                           &srv_desc,
                                           &out_texture.srv);

  if (!SUCCEEDED(check)) {
    LOG_COM(check);
    return false;
  }

  return true;
}

void Renderer::texture_update_subregion(
    Texture2D& texture, 
    u32 subresource, 
    D3D11_BOX* region, 
    void* src_data, 
    u32 src_pitch, 
    u32 src_depth_pitch) {

  context->UpdateSubresource(texture.texture.Get(), subresource, region, src_data, src_pitch, src_depth_pitch);
}

void Renderer::texture_bind(Texture2D * texture) {
  ID3D11ShaderResourceView* psrv[1] = {nullptr}; // clear the 1st slot
  if(texture == nullptr) {
    context->PSSetShaderResources(0, 1, (ID3D11ShaderResourceView**)&psrv); 
  } else { 
    context->PSSetShaderResources(0, 1, texture->srv.GetAddressOf());
  }
}

void Renderer::draw_fullscreen_quad() {
  context->VSSetConstantBuffers(0, 1, this->scene_constant_buffer.GetAddressOf());
  context->PSSetConstantBuffers(0, 1, this->scene_constant_buffer.GetAddressOf());
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  this->pixel_shader_bind(this->ps_fullscreen_quad.Get());
  this->vertex_shader_bind(this->vs_fullscreen_quad.Get());
  context->Draw(4, 0); 
}

