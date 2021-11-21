#ifndef GFX_HELPERS_H
#define GFX_HELPERS_H

#include <wrl/client.h> // Microsoft::WRL::ComPtr<T>
// d3d headers for creating d3d context to render with
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler

#include <imgui.h> // menu creation
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <fmt/format.h> // for error printing

struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
};


class Gfx
{

public:
    /// <summary>
    /// Create a new rendering context for the window
    /// </summary>
    explicit Gfx(HWND hwnd, size_t _width, size_t _height);
    Gfx(const Gfx& Gfx) = delete;
    Gfx& operator=( const Gfx& ) = delete;
    ~Gfx();
    void Render();
    void Resize( LPARAM lParam, WPARAM wParam );

private:
    void PrepareImGui();
    void ShutdownImGui();
    bool SetupDx11();
    void ShutdownDx11();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    /*compile a shader from source that is loaded in memory*/
    bool CompileShader_Mem( const char* szShader,
                            const char* szEntrypoint,
                            const char* szTarget,
                            ID3D10Blob** pBlob );

    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> vertexLayout;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
    D3D11_VIEWPORT m_viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    DirectX::XMMATRIX mOrtho;
    HWND hwnd;
    size_t width, height;
};

#endif // !GFX_HELPERS_H
