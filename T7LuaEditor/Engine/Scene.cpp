//
// Created by coxtr on 12/14/2021.
//

#include "Scene.h"
using namespace DirectX;

Scene2D::Scene2D(Renderer *rhi) : camera(XMConvertToRadians(70.f), 16 / 9.f, 1.0f, 10000.f),
                                  sceneConstants{}
{
    auto device = rhi->GetDevice();
    auto context = rhi->GetContext();
    // here we will create the constant buffers for the camera transform
    // setup our dynamic buffers for storing pre-transformed quads
    // setup our texture atlas resource so we can batch render
    // setup the canvas
    sceneConstants.modelViewProjection = DirectX::XMMatrixIdentity() * camera.GetCameraTransform();
    sceneConstants.viewportSize.x = rhi->GetWidth();
    sceneConstants.viewportSize.y = rhi->GetHeight();

    BuildVertexShader(device, QuadShader, &vertexShader, &layoutPosColorCoord);
    BuildPixelShader(device, QuadShader, &pixelShader);

    // creates a 1280x720 canvas plane
    VertexPosColorTexcoord canvas[4] = {
        {XMFLOAT3(-640.f, -360.f, 0.f), XMFLOAT4(1, 0, 0, 1), XMFLOAT2(1.f, 1.f)}, // left top
        {XMFLOAT3(640.f, -360.f, 0.f), XMFLOAT4(1, 1, 1, 1), XMFLOAT2(0.f, 1.f)},  // right top
        {XMFLOAT3(-640.f, 360.f, 0.f), XMFLOAT4(1, 1, 1, 1), XMFLOAT2(1.f, 0.f)},  // left bottom
        {XMFLOAT3(640.f, 360.f, 0.f), XMFLOAT4(1, 1, 1, 1), XMFLOAT2(0.f, 0.f)},   // right bottom
    };

    int indices[] = {0, 1, 2, 2, 1, 3};

    CreateConstantBuffer<PerSceneConsts>(device, &sceneConstants, &sceneCbuf);
    CreateDynamicVertexBuffer<VertexPosColorTexcoord>(device, &vertexBuff, MaxQuads);
    UpdateDynamicVertexBuffer<VertexPosColorTexcoord>(context, vertexBuff.Get(), canvas, 4);
    CreateDynamicIndexBuffer(device, &indexBuff, MaxIndices);
    UpdateDynamicIndexBuffer(context, indexBuff.Get(), indices, 6);
    quadCount++;
    indexCount += 6;
}

void Scene2D::Resize(LPARAM lparam, WPARAM wparam)
{
    auto width = LOWORD(lparam);
    auto height = HIWORD(lparam);
    camera.SetAspectRatio(float(width) / float(height));
}

void Scene2D::AddUIQuad(ID3D11DeviceContext *context, UIQuad quad, XMMATRIX transform)
{
    auto ent = registery_.create();
    registery_.emplace<XMMATRIX>(ent, transform);
    registery_.emplace<UIQuad>(ent, quad);

    VertexPosColorTexcoord canvas[4] = {
        {XMFLOAT3(-40.f * quadCount, -60.f, 0.f), XMFLOAT4(0, 1, 1, 0.2f), XMFLOAT2(1.f, 1.f)}, // left top
        {XMFLOAT3(40.f, -60.f, 0.f), XMFLOAT4(0, 1, 1, 0.2f), XMFLOAT2(0.f, 1.f)},              // right top
        {XMFLOAT3(-40.f, 60.f, 0.f), XMFLOAT4(0, 1, 1, 0.2f), XMFLOAT2(1.f, 0.f)},              // left bottom
        {XMFLOAT3(40.f, 60.f, 0.f), XMFLOAT4(0, 1, 1, 0.2f), XMFLOAT2(0.f, 0.f)},               // right bottom
    };
    AppendDynamicVertexBuffer<VertexPosColorTexcoord>(context, vertexBuff.Get(), &canvas[0], ARRAYSIZE(canvas), 4 * quadCount);
    int indices[] = {
        0 + int(4 * quadCount),
        1 + int(4 * quadCount),
        2 + int(4 * quadCount),
        2 + int(4 * quadCount),
        1 + int(4 * quadCount),
        3 + int(4 * quadCount)};
    AppendDynamicIndexBuffer(context, indexBuff.Get(), indices, 6, indexCount);
    quadCount++;
    indexCount += 6;
}

void Scene2D::RenderScene(Renderer *rhi, float timestep)
{
    auto context = rhi->GetContext();
    sceneConstants.modelViewProjection = DirectX::XMMatrixIdentity() * camera.GetCameraTransform();
    UpdateConstantBuffer<PerSceneConsts>(context, 0, &sceneConstants, sceneCbuf.Get());
    if (ImGui::Begin("Scene"))
    {
        if (ImGui::Button("Add Image"))
        {
            auto q = UIQuad{0, 100, 0, 100, 0};
            AddUIQuad(rhi->GetContext(), q, DirectX::XMMatrixIdentity());
        }
    }
    ImGui::End();
    // do animation lerp etc
    // we need to bind the shaders
    context->VSSetShader(vertexShader.Get(), nullptr, 0);
    context->PSSetShader(pixelShader.Get(), nullptr, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(layoutPosColorCoord.Get());
    // update constant buffers
    BindConstantBuffer(context, 0, sceneCbuf.Get());
    // bind textures
    // bind vertex buffer
    BindDynamicVertexBuffers<VertexPosColorTexcoord>(context, vertexBuff.GetAddressOf());
    // bind index buffer
    BindDynamicIndexBuffer(context, indexBuff.Get());

    context->DrawIndexed(UINT(6 * quadCount), 0u, 0);
}

void Scene2D::HandleUI()
{
    // if (ImGui::Begin("Scene"))
    // {
    //     if (ImGui::Button("Add Image"))
    //     {
    //         auto q = UIQuad{0, 100, 0, 100, 0};
    //         AddUIQuad(q, DirectX::XMMatrixIdentity());
    //     }
    // }
    // ImGui::End();
}

void Scene2D::HandleInput(Mouse &mouse, Keyboard &kbd)
{
    if (mouse.wheelDelta > 0) // this really needs to be an event...
    {
        ZoomIn(mouse.pos[0], mouse.pos[1], 0.5 * mouse.wheelDelta);
        mouse.wheelDelta = 0;
    }
    else if (mouse.wheelDelta < 0)
    {
        ZoomOut(mouse.pos[0], mouse.pos[1], 0.5f * mouse.wheelDelta);
        mouse.wheelDelta = 0;
    }
    if (mouse.buttons.leftButton)
    {
        camera.Translate(XMFLOAT3(10, 0.0, 0.0));
    }
    if (mouse.buttons.rightButton)
    {
        camera.Translate(XMFLOAT3(-10, 0.0, 0.0));
    }
}

bool Scene2D::IsPointInRect(float const pt[2], float rect[4])
{
    return pt[0] > rect[0] && pt[0] < rect[1] && pt[1] > rect[2] && pt[1] < rect[3];
}

void Scene2D::ZoomIn(float x, float y, float amount)
{
    // we are looking down the Z axis,
    camera.Translate(XMFLOAT3(0, 0, amount));
}

void Scene2D::ZoomOut(float x, float y, float amount)
{
    // we are looking down the Z axis,
    camera.Translate(XMFLOAT3(0, 0, amount));
}
