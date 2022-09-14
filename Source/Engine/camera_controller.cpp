#include "camera_controller.h"
#include "win32_input.h"
#include <imgui.h> // debug only
#include "camera.h"

namespace camera_controller {
    void flycam_fps(float dt, Camera& camera, GameInputMouseState const& new_state) {
        using namespace DirectX;
        static GameInputMouseState last_state{};
        static XMVECTOR default_forward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
        static XMVECTOR default_right = XMVectorSet(1.f, 0.f, 0.f, 0.f);
        XMVECTOR camera_target;
        XMVECTOR camera_offset = XMVectorSet(0, 0, 0, 0);
        XMMATRIX rotation;
        XMMATRIX temp_y_matrix;
        static float movement_speed = 5.0f;

        // process input
        camera.pitch += 0.003 * (float)(new_state.positionY - last_state.positionY);
        camera.yaw   += 0.003 * (float)(new_state.positionX - last_state.positionX);

        rotation = XMMatrixRotationRollPitchYaw(camera.pitch, camera.yaw, camera.roll);
        temp_y_matrix = XMMatrixRotationY(camera.yaw);

        if (input::key_down('W')) {
            camera_offset = XMVectorAdd(camera_offset, XMVectorSet(0.0, 0.0, 1.0, 0.0));
        }

        if (input::key_down('S')) {
            camera_offset = XMVectorAdd(camera_offset, XMVectorSet(0.0, 0.0, -1.0, 0.0));
        }

        if(input::key_down('A')) {
            camera_offset = XMVectorAdd(camera_offset, XMVectorSet(-1.0, 0.0, 0.0, 0.0));
        }

        if(input::key_down('D')) {
            camera_offset = XMVectorAdd(camera_offset, XMVectorSet(1.0, 0.0, 0.0, 0.0));
        }

        camera.origin += movement_speed * XMVector3Normalize(XMVector3TransformCoord(camera_offset, rotation));

        if(input::key_down('E')) camera.origin += movement_speed * XMVectorSet(0.0, 1.0, 0.0, 0.0);
        if(input::key_down('Q')) camera.origin += movement_speed * XMVectorSet(0.0, -1.0, 0.0, 0.0);

        camera_target = XMVector3Normalize(XMVector3TransformCoord(default_forward, rotation));
        camera.forward = XMVector3TransformCoord(default_forward, temp_y_matrix);
        camera.up = XMVector3TransformCoord(camera.up, temp_y_matrix);
        camera.right = XMVector3TransformCoord(default_right, temp_y_matrix);

        camera.update_view(XMMatrixLookAtLH(camera.origin, camera.origin + camera_target, camera.up));
        if(ImGui::Begin("flycam fps debug")) {
            ImGui::Text("time_delta: %f", dt);
            ImGui::Text(
                "mouse delta (%d, %d)", 
                new_state.positionX - last_state.positionX,
                new_state.positionY - last_state.positionY
            );
            ImGui::Separator();
            ImGui::Text(
                "origin: %.2f %.2f %.2f", 
                XMVectorGetX(camera.origin), 
                XMVectorGetY(camera.origin),
                XMVectorGetZ(camera.origin)
            );
            ImGui::Text("pitch: %f | yaw: %f", camera.pitch, camera.yaw);
            ImGui::Text(
                "target: %.2f %.2f %.2f", 
                XMVectorGetX(camera_target), 
                XMVectorGetY(camera_target),
                XMVectorGetZ(camera_target)
            );
        }
        ImGui::End();
        last_state = new_state;
    }
};
