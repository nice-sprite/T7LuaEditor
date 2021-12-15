#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H

#include <wrl/client.h> // Microsoft::WRL::ComPtr<T>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler
#pragma comment( lib, "dxguid.lib")

#include <imgui.h> // menu creation
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <fmt/format.h> // for error printing
#include <vector>
#include "Timer.h"
#include "Texture.h"
#include "ImGUIManager.h"
/*
#include "GfxResource/VertexShader.h"
#include "GfxResource/PixelShader.h"
*/

class VertexShader;
class PixelShader;
#include "Vertex.h"

namespace wrl = Microsoft::WRL;

class Renderer {
    friend class GraphicsResource; // this is where it started to stink...

public:
    /// <summary>
    /// Create a new rendering context for the window
    /// </summary>
    explicit Renderer(HWND hwnd, size_t _width, size_t _height);

    Renderer(const Renderer &Gfx) = delete;

    Renderer &operator=(const Renderer &) = delete;

    ~Renderer();

    void Render();

    void Resize(LPARAM lParam, WPARAM wParam);

    DirectX::XMMATRIX &GetOrthoMatrix() { return orthoMat; }

    DirectX::XMMATRIX &GetProjectionMatrix() { return projectionMat; }

    DirectX::XMMATRIX &GetWorldMatrix() { return worldMat; }

    [[nodiscard]] size_t GetWidth() const { return width; }

    [[nodiscard]] size_t GetHeight() const { return height; }

private:

    bool SetupDx11();

    void CreateRenderTarget();

    void CleanupRenderTarget();

    void DrawTestImage();

    struct ConstantBuffer {
        float time;
    };

private:
    std::unique_ptr<ImGUIManager> imgui_;
    wrl::ComPtr<ID3D11Device> device;
    wrl::ComPtr<ID3D11DeviceContext> context;
    wrl::ComPtr<ID3D11RenderTargetView> renderTargetView;
    wrl::ComPtr<IDXGISwapChain> swapChain;
    wrl::ComPtr<ID3D11RasterizerState> rasterizerState;
    wrl::ComPtr<ID3D11BlendState> blendState_;
    wrl::ComPtr<ID3D11Buffer> constantBuf_;
    D3D11_VIEWPORT viewport{};
    DirectX::XMMATRIX orthoMat{};
    DirectX::XMMATRIX projectionMat{};
    DirectX::XMMATRIX worldMat{};
    DirectX::XMFLOAT4 backgroundColor_;
    HWND hwnd;
    size_t width, height;
    Timer frameTimer;
    Texture demoTexture;
};

#endif // !GFX_HELPERS_H
