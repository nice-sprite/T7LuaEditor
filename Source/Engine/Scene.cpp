//
// Created by coxtr on 12/14/2021.
//

#include "Scene.h"
#include <DirectXMath.h>
#include "Texture.h"
using namespace DirectX;
using namespace std::string_literals;

Scene2D::Scene2D(Renderer *rhi) : camera(XMConvertToRadians(70.f), 16 / 9.f, 1.0f, 10000.f),
                                  sceneConstants{}
{
    auto device = rhi->GetDevice();
    auto context = rhi->GetContext();
    // here we will create the constant buffers for the camera transform
    // setup our dynamic buffers for storing pre-transformed quads
    // setup our texture atlas resource so we can batch render
    // setup the canvas
    sceneConstants.viewportSize.x = rhi->GetWidth();
    sceneConstants.modelViewProjection = DirectX::XMMatrixIdentity() * camera.GetCameraTransform();
    sceneConstants.viewportSize.y = rhi->GetHeight();

    build_vertex_shader(device, QuadShader, &vertexShader, &layoutPosColorCoord);
    BuildPixelShader(device, QuadShader, &pixelShader);

    // creates a 1280x720 canvas plane
    VertexPosColorTexcoord canvas[4] = {
        {XMFLOAT3(-640.f, -360.f, 0.f), XMFLOAT4(1, 0, 0, 1), XMFLOAT2(1.f, 1.f)}, // left top
        {XMFLOAT3(640.f, -360.f, 0.f), XMFLOAT4(1, 1, 1, 1), XMFLOAT2(0.f, 1.f)},  // right top
        {XMFLOAT3(-640.f, 360.f, 0.f), XMFLOAT4(1, 1, 1, 1), XMFLOAT2(1.f, 0.f)},  // left bottom
        {XMFLOAT3(640.f, 360.f, 0.f), XMFLOAT4(1, 1, 1, 1), XMFLOAT2(0.f, 0.f)},   // right bottom
    };

    int indices[] = {0, 1, 2, 2, 1, 3};

    create_constant_buffer<PerSceneConsts>(device, &sceneConstants, &sceneCbuf);
    create_dynamic_vertex_buffer<VertexPosColorTexcoord>(device, &vertexBuff, MaxQuads);
    update_dynamic_vertex_buffer<VertexPosColorTexcoord>(context, vertexBuff.Get(), canvas, 4);
    create_dynamic_index_buffer(device, &indexBuff, MaxIndices);
    update_dynamic_index_buffer(context, indexBuff.Get(), indices, 6);
    quadCount++;
    indexCount += 6;

    auto mouse_cb = [this](float x, float y, WPARAM flags) -> bool
    {
        static int callcount = 0;
        ImGui::Text("cursor (%f, %f)", x, y);
        ImGui::Text("wparam: %d", flags);
        ImGui::Text("Calls: %d", callcount);
        auto ctrl = Input::Ctrl(flags),
             shift = Input::Shift(flags),
             btn_left = Input::Btn_Left(flags),
             btn_right = Input::Btn_Right(flags),
             btn_middle = Input::Btn_Mid(flags),
             xbtn1 = Input::Btn_XBtn1(flags),
             xbtn2 = Input::Btn_XBtn2(flags);

        ImGui::Text("ctrl: %d\n", ctrl);
        ImGui::Text("shift: %d\n", shift);
        ImGui::Text("btn_left: %d\n", btn_left);
        ImGui::Text("btn_right: %d\n", btn_right);
        ImGui::Text("btn_mid: %d\n", btn_middle);
        ImGui::Text("xbtn1: %d\n", xbtn1);
        ImGui::Text("xbtn2: %d\n", xbtn2);
        ++callcount;
        return true;
    };

    auto kbd_cb = [this](Input::keyboard_t &keyState) -> bool
    {
        std::string pressedKeys = "";
        for (auto c : keyState.key)
        {
            if (c != 0)
                pressedKeys += c;
        }
        ImGui::Text("%s", pressedKeys.c_str());
        return true;
    };
    Input::RegisterMouseMove(mouse_cb);
    Input::RegisterKeyboardFn(kbd_cb);
}

void Scene2D::Resize(LPARAM lparam, WPARAM wparam)
{
    viewportWidth = LOWORD(lparam);
    viewportHeight = HIWORD(lparam);

    camera.SetAspectRatio(float(viewportWidth) / float(viewportHeight));
}

void Scene2D::AddUIQuad(ID3D11DeviceContext *context, UIQuad quad, XMMATRIX transform)
{
    // auto ent = registery_.create();
    // registery_.emplace<DirectX::XMMATRIX>(ent, transform);
    // registery_.emplace<UIQuad>(ent, quad);

    VertexPosColorTexcoord canvas[4] = {
        {XMFLOAT3(-40.f , -60.f, 0.f), XMFLOAT4(0, 1, 1, 0.2f), XMFLOAT2(1.f, 1.f)}, // left top
        {XMFLOAT3(40.f, -60.f, 0.f), XMFLOAT4(0, 1, 1, 0.2f), XMFLOAT2(0.f, 1.f)},              // right top
        {XMFLOAT3(-40.f, 60.f, 0.f), XMFLOAT4(0, 1, 1, 0.2f), XMFLOAT2(1.f, 0.f)},              // left bottom
        {XMFLOAT3(40.f, 60.f, 0.f), XMFLOAT4(0, 1, 1, 0.2f), XMFLOAT2(0.f, 0.f)},               // right bottom
    };
    append_dynamic_vertex_buffer<VertexPosColorTexcoord>(context, vertexBuff.Get(), &canvas[0], ARRAYSIZE(canvas), 4 * quadCount);
    int indices[] = {
        0 + int(4 * quadCount),
        1 + int(4 * quadCount),
        2 + int(4 * quadCount),
        2 + int(4 * quadCount),
        1 + int(4 * quadCount),
        3 + int(4 * quadCount)};
    append_dynamic_index_buffer(context, indexBuff.Get(), indices, 6, indexCount);
    quadCount++;
    indexCount += 6;
}

void Scene2D::RenderScene(Renderer *rhi, float timestep)
{
    auto context = rhi->GetContext();
    sceneConstants.modelViewProjection = DirectX::XMMatrixIdentity() * camera.GetCameraTransform();
    update_constant_buffer<PerSceneConsts>(context, 0, &sceneConstants, sceneCbuf.Get());
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
    bind_constant_buffer(context, 0, sceneCbuf.Get());
    // bind textures
    // bind vertex buffer
    bind_dynamic_vertex_buffers<VertexPosColorTexcoord>(context, vertexBuff.GetAddressOf());
    // bind index buffer
    bind_dynamic_index_buffer(context, indexBuff.Get());

    context->DrawIndexed(UINT(6 * quadCount), 0u, 0);
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

// // https://antongerdelan.net/opengl/raycasting.html
// std::vector<entt::entity> Scene2D::Raycast(float viewportX, float viewportY)
// {
//     using namespace DirectX;
//     auto projection = camera.GetProjection();
//     auto projDet = XMMatrixDeterminant(projection);
//     auto invProj = XMMatrixInverse(&projDet, projection);
//     auto view = camera.GetView();
//     auto x = (2.f * viewportX) / viewportWidth - 1.f;
//     auto y = 1.f - (2.f * viewportY) / viewportHeight;
//     auto z = 1.f;

//     return {};
// }
