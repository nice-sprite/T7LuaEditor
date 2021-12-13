//
// Created by coxtr on 11/25/2021.
//

#include "DrawMe.h"
constexpr auto DefaultShader = L"C:\\Users\\coxtr\\source\\repos\\T7LuaEditor\\T7LuaEditor\\ShaderSrc";
DrawMe::DrawMe(Renderer& gfx) /*:*/
/*
    vertexShader(gfx, DefaultShader), pixelShader(gfx, DefaultShader),
    inputLayout(gfx, vertexShader)
*/
{
/*
    std::vector<Vertex> model {};
    Vertex bottomLeft{}, topMiddle{}, bottomRight{};
    DirectX::XMFLOAT4 green = DirectX::XMFLOAT4(0.f, 0.5f, 0.1f, 1.0f);
    bottomLeft.position = DirectX::XMFLOAT3(-1.f, -1.f, 0.f);
    bottomLeft.color = green;

    topMiddle.position = DirectX::XMFLOAT3(0.f, 1.f, 0.f);
    topMiddle.color = green;

    bottomRight.position = DirectX::XMFLOAT3(1.f, -1.f, 0.f);
    bottomRight.color = green;

    model.push_back(bottomLeft);
    model.push_back(topMiddle);
    model.push_back(bottomRight);

    std::vector<unsigned long> indexList{0, 1, 2};

    vertexBuffer = VertexBuffer{gfx, model};
    indexBuffer = IndexBuffer{ gfx, indexList};
*/
}

void DrawMe::Draw(Renderer& gfx)
{
/*
    vertexBuffer.Bind(gfx);
    indexBuffer.Bind(gfx);
    inputLayout.Bind(gfx);
    vertexShader.Bind(gfx);
    pixelShader.Bind(gfx);
*/
}
