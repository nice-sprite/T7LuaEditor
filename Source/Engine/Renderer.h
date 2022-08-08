#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H
#include "gpu_resources.h"
#include "render_graph.h"
#include <d2d1_3.h>
#include <d3d11_4.h>
#include <dwrite_3.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXMath.h>

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

    /*
     * given a description of all the resources a pass 
     * depends on, will take care of initializing the resources and 
     * returns a structure with references to the created GPU resources
     */
    //void create_render_pass_resources(char* passKey, PassDependencies dependencies, RenderPassResources* outPass);

private:
    bool initialize_d3d();
    bool initialize_imgui();
    void create_backbuffer_view();
    void reset_backbuffer_views();

public: 
    float width, height;
    RenderGraph renderGraph;

private:

    ComPtr<ID3D11Device5> device;
    ComPtr<ID3D11DeviceContext4> context;
    ComPtr<IDXGISwapChain4> swapChain;
    // D3D
    D3D11_VIEWPORT viewport{};
    ComPtr<ID3D11BlendState> alphaBlendState;
    ComPtr<ID3D11RasterizerState2> rasterizerState;
    ComPtr<ID3D11RenderTargetView> rtv;
    ComPtr<IDXGIFactory7> dxgiFactory;
    ComPtr<ID3D11SamplerState> gridSS;


    DirectX::XMFLOAT4 clearColor;
    HWND hwnd;
};

#endif // !GFX_HELPERS_H
