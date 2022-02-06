//
// Created by coxtr on 12/14/2021.
//

#ifndef T7LUAEDITOR_SCENE_H
#define T7LUAEDITOR_SCENE_H

#include "Camera.h"
#include "Renderer.h"
#include "./GfxResource/ConstantBuffer.h"
#include "./GfxResource/VertexBuffer.h"
#include <imgui.h>
#include "Input.h"
#include <vector>
#include <entt/entt.hpp>
#include <wrl/client.h>
#include "Texture.h"

using namespace Microsoft::WRL;

struct UIQuad
{
    float left, right, top, bottom;
    int texture;
};

struct VertexPosColorTexcoord
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
    DirectX::XMFLOAT2 texcoord;
};
// Scene:
// - Scene contains all the geometry/layout information for a scene
// - responsible for sending vertex data to renderer

class Scene2D
{
    static constexpr auto MaxQuads = 20000;
    static constexpr auto MaxIndices = 6 * 20000; // there are 6 indices per quad
    static constexpr auto QuadShader = L"w:/Priscilla/Source/HLSL/TexturedQuad.hlsl";
    size_t quadCount = 0;
    size_t indexCount = 0;
    std::vector<UIQuad> quads_;
    ComPtr<ID3D11Buffer> vertexBuff;
    ComPtr<ID3D11Buffer> indexBuff;
    ComPtr<ID3D11InputLayout> layoutPosColorCoord;
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;

public:

    // BUFFER TYPES
    __declspec(align(16))
    struct PerSceneConsts
    {
        DirectX::XMMATRIX modelViewProjection; // 64 bytes
        // x: current time, in milliseconds
        // y: current tick number
        // z: delta from last tick
        // w: frame number (or unused)
        DirectX::XMFLOAT4 timeTickDeltaFrame; // 16 bytes
        DirectX::XMFLOAT2 viewportSize; // 8 bytes
        DirectX::XMFLOAT2 windowSize; // 8 bytes
    };

    Scene2D(Renderer* rhi);

    void AddUIQuad(UIQuad quad);
    void AddUIQuad(ID3D11DeviceContext* context, UIQuad quad, DirectX::XMMATRIX transform);

    void RenderScene(Renderer *rhi, float timestep);
    void HandleUI();
    void Resize(LPARAM lparam, WPARAM wparam);
    void ZoomIn(float x, float y, float amount);
    void ZoomOut(float x, float y, float amount);

    // std::vector<entt::entity> Raycast(float x, float y);

private:
    bool IsPointInRect(float const pt[2], float rect[4]);

private:
    PerSceneConsts sceneConstants;
    ComPtr<ID3D11Buffer> sceneCbuf;
    Camera camera;
    UINT viewportWidth, viewportHeight;
    // entt::registry registery_;
};

#endif //T7LUAEDITOR_SCENE_H
