//
// Created by coxtr on 12/14/2021.
//

#ifndef T7LUAEDITOR_SCENE_H
#define T7LUAEDITOR_SCENE_H
#include "camera.h"
#include "gpu_resources.h"
#include "win32_input.h"
#include "render_graph.h"
#include <entt/entt.hpp>
#include <imgui.h>
#include <vector>

struct UIQuad
{
    float left, right, top, bottom;
    int texture;
};



class Scene {

public:
    static constexpr auto MaxQuads = 10000;
    static constexpr auto MaxIndices = 6 * MaxQuads; // there are 6 indices per quad
    static constexpr auto QuadShader = L"w:/Priscilla/Source/HLSL/TexturedQuad.hlsl";

    __declspec(align(16))
    struct PerSceneConsts
    {
        DirectX::XMMATRIX modelViewProjection; // 64 bytes
        DirectX::XMFLOAT4 timeTickDeltaFrame; // 16 bytes
        DirectX::XMFLOAT2 viewportSize; // 8 bytes
        DirectX::XMFLOAT2 windowSize; // 8 bytes
    };

    Scene();

    void add_quad(float left, 
        float right, 
        float top, 
        float bottom, 
        int texture = 0
    );

    void draw_scene(float timestep);

public:
    size_t quadCount = 0;
    std::array<UIQuad, MaxQuads> quads; 
    
private:
    PerSceneConsts sceneConstants;
    int width, height;
};

#endif //T7LUAEDITOR_SCENE_H
