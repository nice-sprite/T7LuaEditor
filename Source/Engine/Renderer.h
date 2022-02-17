#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H
#include "gpu_resources.h"
#include "Vertex.h"
#include <d2d1_3.h>
#include <d3d11_4.h>
#include <dwrite_3.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

class Renderer
{
public:
    explicit Renderer(HWND hwnd, float _width, float _height);
    ~Renderer();
    Renderer(const Renderer &Gfx) = delete;
    Renderer &operator=(const Renderer &) = delete;
    void set_and_clear_backbuffer();
    void present();
    void resize_swapchain_backbuffer(int width, int height, bool minimized);
    void imgui_frame_begin();
    void imgui_frame_end();

private:
    bool initialize_d3d();
    bool initialize_d2d_interop();
    bool initialize_imgui();
    void create_backbuffer_view();
    void reset_backbuffer_views();

public: 
    float width, height;

private:

    ComPtr<ID3D11InputLayout> quadVertLayout_;
    ComPtr<ID3D11Device5> device;
    ComPtr<ID3D11DeviceContext4> context;
    // D3D
    D3D11_VIEWPORT viewport{};
    ComPtr<ID3D11BlendState> alphaBlendState;
    ComPtr<ID3D11RasterizerState2> rasterizerState_;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<IDXGISwapChain4> swapChain;
    ComPtr<IDXGIFactory7> dxgiFactory;
    ComPtr<ID3D11SamplerState> gridSS;

    // D2D
    // TODO: remove
    ComPtr<ID2D1Device6> device2D;
    ComPtr<ID2D1DeviceContext6> context2D;
    ComPtr<ID2D1RenderTarget> rtv2D;
    ComPtr<IDWriteFactory7> DWriteFactory;
    ComPtr<IDWriteTextFormat> defaultDWTextFormat;
    ComPtr<ID2D1SolidColorBrush> solidWhiteBrush2D;

    DirectX::XMFLOAT4 clearColor;
    HWND hwnd;
};

#endif // !GFX_HELPERS_H
