#include "Application.h"
#include <Windows.h>
#include <imgui.h>
#include <Tracy.hpp> 
#include "../Engine/camera_controller.h"
namespace app 
{
    void start(HINSTANCE hinst, const wchar_t *appname)
    {
        input::start_game_input();
        mainWindow = win32::create_window(hinst, appname, L"luaeditor", AppWidth, AppHeight, win32_message_callback, AppIcon);
        auto rect = mainWindow.clientRect;

        scene = new Scene();  

        rhi = new Renderer(mainWindow.hwnd, float(rect.right - rect.left), float(rect.bottom - rect.top)); 
        // DEBUG SELECTION RECT
        rhi->add_selection_rect(-50.f, 50.f, -50.f, 50.f);
        rhi->create_world_grid();
        SetWindowPos(mainWindow.hwnd, nullptr, 0, 0,
                     (rect.right - rect.left + 1), // the + 1 is because the WM_SIZE message doesn't go through if the size is the same
                     (rect.bottom - rect.top),
                     SWP_NOMOVE);

        /* INPUT DEBUGGING and TESTING */
        input::register_callback([](input::MouseState const& mouse, input::KeyboardState const& kbd) -> bool {
            if(ImGui::Begin("Keyboard Debug")) {
                ImGui::Text("keyboard state debug");
                ImGui::Text("shift_down, %d", kbd.shift_down);
                ImGui::Text("tab_down, %d", kbd.tab_down);
                ImGui::Text("backspace_down, %d", kbd.backspace_down);
                ImGui::Text("enter_down, %d", kbd.enter_down);
                ImGui::Text("space_down, %d", kbd.space_down);
                ImGui::Text("ctrl_down, %d", kbd.ctrl_down);
                ImGui::Text("caps_down, %d", kbd.caps_down);
                size_t index = 0;
                for(input::KeyState const& key : kbd.keys) {
                    char value = key.character_value ? key.character_value : (char)index;
                    ImGui::Text("%c | h: %d | d: %d", value, (int)key.held, (int)key.down);
                    index++;
                }
            }
            ImGui::End();

            if(ImGui::Begin("Mouse Debug")) {
                ImGui::Text("%f %f", mouse.x, mouse.y);
                ImGui::Separator();
                ImGui::Text("scroll: %d", mouse.scroll_delta);
                ImGui::Separator();

                ImGui::Text("left_button: %d", mouse.left_down);
                ImGui::Text("left_double: %d", mouse.left_double_click);
                ImGui::Separator();

                ImGui::Text("right_button: %d", mouse.right_down);
                ImGui::Text("right_double: %d", mouse.right_double_click);
                ImGui::Separator();

                ImGui::Text("middle_button: %d", mouse.middle_down);
                ImGui::Text("middle_double: %d", mouse.middle_double_click);
                ImGui::Separator();

                ImGui::Text("extra buttons");
                ImGui::Text("x1_down: %d", mouse.x1_down);
                ImGui::Text("x1_double: %d", mouse.x1_double_click);

                ImGui::Text("\nx2_down: %d", mouse.x2_down);
                ImGui::Text("x2_double: %d", mouse.x2_double_click);
            }
            ImGui::End();

            static float cursor_down_pos_x = 0.f; 
            static float cursor_down_pos_y = 0.f; 
            static bool held = false;
            if(mouse.left_down && !held) {
                cursor_down_pos_x = mouse.x; 
                cursor_down_pos_y = mouse.y; 
                held = true;
            }else if(mouse.left_down && held) {
                rhi->imgui_draw_screen_rect(cursor_down_pos_x, mouse.x, cursor_down_pos_y, mouse.y);
            } else {
                held = false;
            }

            //rhi->imgui_draw_screen_rect(-50.f, 50.f, -50.f, 50.f);
            return true;
        });


    }

    void draw_scene(float ts) {
        ZoneScoped("");
        std::vector<VertexPosColorTexcoord> upload_quads{scene->quadCount * 4};
        std::vector<int> upload_indices{};
        upload_indices.resize(scene->quadCount * 6);

        // tesselate the quads
        for(int i = 0; i < scene->quadCount; ++i) {
            auto q = scene->quads[i];

            VertexPosColorTexcoord quad_verts[] = {
                {
                    DirectX::XMFLOAT3{q.left, q.top, 0.0f},
                    DirectX::XMFLOAT4{0.9, 0.9, 0.9, 1.0},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                },   
                {
                    DirectX::XMFLOAT3{q.right, q.top, 0.0f},
                    DirectX::XMFLOAT4{0.9, 0.9, 0.9, 1.0},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                },
                {
                    DirectX::XMFLOAT3{q.left, q.bottom, 0.0f},
                    DirectX::XMFLOAT4{0.9, 0.9, 0.9, 1.0},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                }, 
                {
                    DirectX::XMFLOAT3{q.right, q.bottom, 0.0f},
                    DirectX::XMFLOAT4{0.9, 0.9, 0.9, 1.0},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                },   
            };

            upload_quads[i * 4 + 0] = quad_verts[0];
            upload_quads[i * 4 + 1] = quad_verts[1];
            upload_quads[i * 4 + 2] = quad_verts[2];
            upload_quads[i * 4 + 3] = quad_verts[3];


            upload_indices[i * 6 + 0] = i * 4 + 2;
            upload_indices[i * 6 + 1] = i * 4 + 3;
            upload_indices[i * 6 + 2] = i * 4 + 1;
            upload_indices[i * 6 + 3] = i * 4 + 2;
            upload_indices[i * 6 + 4] = i * 4 + 1;
            upload_indices[i * 6 + 5] = i * 4 + 0;
        }

        update_dynamic_vertex_buffer(
            rhi->context.Get(),
            rhi->scene_vertex_buffer.Get(), 
            (void*)(upload_quads.data()),
            sizeof(VertexPosColorTexcoord) * upload_quads.size()
        );

        update_dynamic_index_buffer(
            rhi->context.Get(), 
            rhi->scene_index_buffer.Get(), 
            (upload_indices.data()),
            upload_indices.size()
        );

        bind_dynamic_vertex_buffers(
            rhi->context.Get(),
            rhi->scene_vertex_buffer.GetAddressOf(),
            sizeof(VertexPosColorTexcoord),
            0
        );

        bind_dynamic_index_buffer(
            rhi->context.Get(),
            rhi->scene_index_buffer.Get()
        );

        rhi->scene_consts.modelViewProjection = DirectX::XMMatrixIdentity() * rhi->camera.get_transform();
        update_constant_buffer(
            rhi->context.Get(),
            0,
            (void*)&rhi->scene_consts,
            sizeof(rhi->scene_consts),
            rhi->scene_constant_buffer.Get()
        );

        bind_constant_buffer(rhi->context.Get(), 0, rhi->scene_constant_buffer.Get());
        rhi->context->IASetInputLayout(rhi->vtx_pos_color_tex_il.Get());
        rhi->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        rhi->context->VSSetShader(rhi->scene_vertex_shader.Get(), NULL, NULL);
        rhi->context->PSSetShader(rhi->scene_pixel_shader.Get(), NULL, NULL);
        rhi->context->DrawIndexed(upload_indices.size(), 0, 0);
        ImGui::Text("num_quads: %d", scene->quadCount);
    }


    void tick(float timestep)
    {
        static const char* sl_FrameTick = "Tick";
        FrameMarkStart(sl_FrameTick);

        rhi->set_and_clear_backbuffer();
        rhi->imgui_frame_begin();

        if(!(ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard)) {
            input::process_input_for_frame();
        }

        auto mouse = input::poll_mouse();
        auto gamepad = input::poll_gamepad();
        input::poll_keys();

        // DEBUG GAMEINPUT
        if(ImGui::Begin("GAMEINPUT DEBUG")) {
            float left_stick[2] = {gamepad.leftThumbstickX, gamepad.leftThumbstickY};
            float right_stick[2] = {gamepad.rightThumbstickX, gamepad.rightThumbstickY};
            ImGui::Text("MOUSE");
            ImGui::Text("deltas: %d %d", mouse.positionX, mouse.positionY);
            ImGui::Text("wheel: %d %d", mouse.wheelX, mouse.wheelY);
            ImGui::Text("buttons: %d %d %d %d %d %d %d",
                    mouse.buttons & GameInputMouseLeftButton, 
                    mouse.buttons & GameInputMouseRightButton,
                    mouse.buttons & GameInputMouseMiddleButton,
                    mouse.buttons & GameInputMouseButton4,
                    mouse.buttons & GameInputMouseButton5,
                    mouse.buttons & GameInputMouseWheelTiltLeft,
                    mouse.buttons & GameInputMouseWheelTiltRight
            );
            ImGui::Separator();

            ImGui::Text("Controlla Playa?!?!?!");
            ImGui::SliderFloat("LT", &gamepad.leftTrigger, 0.0f, 1.0f, "%.2f", 1.0f);
            ImGui::SliderFloat("RT", &gamepad.rightTrigger, 0.0f, 1.0f, "%.2f", 1.0f);
            ImGui::DragFloat2("Left Stick", left_stick);
            ImGui::DragFloat2("Right Stick", right_stick);

            ImGui::Text(
                "None %d\n,  Menu %d\n,  View %d\n,  A %d\n,  B %d\n,  X %d\n,  Y %d\n,  DPadUp %d\n,  DPadDown %d\n,  DPadLeft %d\n,  DPadRight %d\n,  LeftShoulder %d\n,  RightShoulder %d\n,  LeftThumbstick %d\n,  RightThumbstick %d", 
                gamepad.buttons & GameInputGamepadNone ,  
                gamepad.buttons & GameInputGamepadMenu ,  
                gamepad.buttons & GameInputGamepadView ,  
                gamepad.buttons & GameInputGamepadA ,  
                gamepad.buttons & GameInputGamepadB ,  
                gamepad.buttons & GameInputGamepadX ,  
                gamepad.buttons & GameInputGamepadY ,  
                gamepad.buttons & GameInputGamepadDPadUp ,  
                gamepad.buttons & GameInputGamepadDPadDown ,  
                gamepad.buttons & GameInputGamepadDPadLeft ,  
                gamepad.buttons & GameInputGamepadDPadRight ,  
                gamepad.buttons & GameInputGamepadLeftShoulder ,  
                gamepad.buttons & GameInputGamepadRightShoulder ,  
                gamepad.buttons & GameInputGamepadLeftThumbstick ,  
                gamepad.buttons & GameInputGamepadRightThumbstick
            );

            ImGui::Separator();
            ImGui::Text("KEYBOARD");
            ImGui::Text("max_keys: %llu", input::max_keys);
            ImGui::Text("active_keys: %llu", input::active_keys);
            if(input::key_down('W')) ImGui::Text("Forward");
            if(input::key_down('A')) ImGui::Text("Left");
            if(input::key_down('S')) ImGui::Text("Back");
            if(input::key_down('D')) ImGui::Text("Right");
        } 
        ImGui::End();

        static bool show = false;
        ImGui::ShowDemoWindow(&show);

        camera_controller::flycam_fps(timestep, rhi->camera, mouse);
        

        draw_scene(timestep);
        rhi->draw_selection_rect();
        rhi->draw_debug_lines();

        rhi->imgui_frame_end();
        rhi->present();

        FrameMarkEnd(sl_FrameTick);
    }

    LRESULT CALLBACK win32_message_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
            return true;
        switch (msg)
        {
            case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            }
            case WM_MOUSEWHEEL:
            case WM_MOUSEMOVE:

            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK:

            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:

            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK:

            case WM_XBUTTONUP:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONDBLCLK:
            {
                input::cache_mouse_input_for_frame(hwnd, msg, wparam, lparam); 
                break;
            }
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                input::cache_keyboard_input_for_frame(hwnd, msg, wparam, lparam);
                break;
            }
            case WM_SIZE:
            {
                if (rhi)
                {
                    auto newWidth = LOWORD(lparam);
                    auto newHeight = HIWORD(lparam);
                    auto wasMini = wparam == SIZE_MINIMIZED;
                    rhi->resize_swapchain_backbuffer(newWidth, newHeight, wasMini);
                }
                break;
            }

        }

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    void message_loop()
    {
        start_timer(&frameTimer);
        MSG msg;
        bool shouldClose = false;
        while (!shouldClose)
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
                if (msg.message == WM_QUIT)
                {
                    shouldClose = true;
                    break;
                }
            }
            tick(get_timer_ms(&frameTimer));
        }
        input::shutdown_game_input();
    }
    // this is where the program exits: 
    // TODO: call shutdown and free memory


};
