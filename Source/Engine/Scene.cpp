//
// Created by coxtr on 12/14/2021.
//

#include "Scene.h"
#include <DirectXMath.h>
#include "Texture.h"
#include "shader_util.h"
using namespace DirectX;
using namespace std::string_literals;

Scene::Scene() : sceneConstants{}
{
    passDef.vertexShaderPath = (wchar_t*)L"w:/priscilla/hlsl/TexturedQuad.hlsl";
    passDef.pixelShaderPath  = (wchar_t*)L"w:/priscilla/hlsl/TexturedQuad.hlsl";

    passDef.il = new D3D11_INPUT_ELEMENT_DESC[3];
    passDef.il[0] = {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}; // 3 * 4
    passDef.il[1] = {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0};//+4 * 4
    passDef.il[2] = {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0};   //+2 * 4

    passDef.ilSize = 3;
    passDef.constantBufferSize = sizeof(PerSceneConsts);
    passDef.vertexBufferSize = 36 * MaxQuads; // 36 bytes per vertex, 
    passDef.indexBufferSize = MaxIndices; // this is 6 * MaxQuads
    passDef.atlasSlot = 0;
}

void Scene::add_quad(float left, 
        float right, 
        float top, 
        float bottom, 
        int texture )
{
    quads[quadCount] = UIQuad{left, right, top, bottom, texture};
    ++quadCount;
}
