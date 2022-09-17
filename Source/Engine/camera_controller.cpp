#include "camera_controller.h"
#include "win32_input.h"
#include <imgui.h> // debug only
#include "camera.h"

namespace camera_controller {
    inline float min_float(float a, float b) { return a <= b ? a : b; }
    inline float max_float(float a, float b) { return a >= b ? a : b; }
    inline float clamp_float(float a, float low, float high) { return min_float(high, max_float(a, low)); }

    void flycam_fps(float dt, Camera& camera) {
        using namespace DirectX;
        static XMVECTOR default_forward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
        static XMVECTOR default_right = XMVectorSet(1.f, 0.f, 0.f, 0.f);
        XMVECTOR camera_target;
        XMVECTOR camera_offset = XMVectorSet(0, 0, 0, 0);
        XMMATRIX rotation;
        XMMATRIX temp_y_matrix;
        static float movement_speed = 5.0f;

        camera.pitch += 0.003 * (float)(Input::GameInput::mouse_delta().dy);
        camera.yaw   += 0.003 * (float)(Input::GameInput::mouse_delta().dx);

        camera.pitch = clamp_float(camera.pitch, (float)( -XM_PIDIV2 + 0.001 ), (float)( XM_PIDIV2 - 0.001 ));

        rotation = XMMatrixRotationRollPitchYaw(camera.pitch, camera.yaw, camera.roll);
        temp_y_matrix = XMMatrixRotationY(camera.yaw);

        if (Input::GameInput::key_down('W')) {
            camera_offset = XMVectorAdd(camera_offset, XMVectorSet(0.0, 0.0, 1.0, 0.0));
        }

        if (Input::GameInput::key_down('S')) {
            camera_offset = XMVectorAdd(camera_offset, XMVectorSet(0.0, 0.0, -1.0, 0.0));
        }

        if(Input::GameInput::key_down('A')) {
            camera_offset = XMVectorAdd(camera_offset, XMVectorSet(-1.0, 0.0, 0.0, 0.0));
        }

        if(Input::GameInput::key_down('D')) {
            camera_offset = XMVectorAdd(camera_offset, XMVectorSet(1.0, 0.0, 0.0, 0.0));
        }

        camera.origin += movement_speed * XMVector3Normalize(XMVector3TransformCoord(camera_offset, rotation));

        if(Input::GameInput::key_down('E')) camera.origin += movement_speed * XMVectorSet(0.0, 1.0, 0.0, 0.0);
        if(Input::GameInput::key_down('Q')) camera.origin += movement_speed * XMVectorSet(0.0, -1.0, 0.0, 0.0);

        camera_target = XMVector3Normalize(XMVector3TransformCoord(default_forward, rotation));
        camera.forward = XMVector3TransformCoord(default_forward, temp_y_matrix);
        camera.up = XMVector3TransformCoord(camera.up, temp_y_matrix);
        camera.right = XMVector3TransformCoord(default_right, temp_y_matrix);
        camera.update_view(XMMatrixLookAtLH(camera.origin, camera.origin + camera_target, camera.up));

#ifdef DEBUG_CAMERA
        if(ImGui::Begin("flycam fps debug")) {
            ImGui::Text("time_delta: %f", dt);
            ImGui::Text(
                "mouse delta (%d, %d)", 
                Input::GameInput::mouse_delta().dx,
                Input::GameInput::mouse_delta().dy
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
#endif
    }


    void dollycam(float dt, Camera& camera) {
        using namespace DirectX;
        static XMVECTOR default_forward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
        static XMVECTOR default_right = XMVectorSet(1.f, 0.f, 0.f, 0.f);
        static float movement_speed = 10.0;  
        XMMATRIX rotation;
        XMVECTOR camera_offset = XMVectorSet(0, 0, 0, 0);
        XMVECTOR camera_target;

        rotation = XMMatrixRotationRollPitchYaw(camera.pitch, camera.yaw, camera.roll);
        if(Input::GameInput::key_down(VK_LCONTROL)) {
            camera_offset += XMVectorSet( 0.0, 0.0, movement_speed * dt * (float)(Input::GameInput::mouse_delta().wheel_y), 0.0);
            if(Input::GameInput::mouse_button_down(GameInputMouseLeftButton)) {
                camera_offset += XMVectorSet(
                    -(float)(Input::GameInput::mouse_delta().dx),
                    -(float)(Input::GameInput::mouse_delta().dy),
                    0.0, 
                    0.0
                );
            }
        }
        camera.origin += XMVector3TransformCoord(camera_offset, rotation);
        camera_target = XMVector3Normalize(XMVector3TransformCoord(default_forward, rotation));
        camera.update_view(XMMatrixLookAtLH(camera.origin, camera.origin + camera_target, camera.up));

#ifdef DEBUG_CAMERA
        if(ImGui::Begin("dollycam debug")) {
            ImGui::SliderFloat("zoom sensitivity", &movement_speed, 0.0f, 10000.0f);
            ImGui::Text("time_delta: %f", dt);
            ImGui::Text(
                "mouse delta (%d, %d)", 
                Input::GameInput::mouse_delta().dx,
                Input::GameInput::mouse_delta().dy
            );

            ImGui::Text("scroll: %d %d", 
                Input::GameInput::mouse_delta().wheel_x,
                Input::GameInput::mouse_delta().wheel_y
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
#endif
    }

};
