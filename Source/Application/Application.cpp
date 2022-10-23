#include "Application.h"
#include <Windows.h>
#include <imgui.h>
#include <Tracy.hpp> 
#include "../Engine/camera_controller.h"
#include "../Engine/files.h"




namespace app 
{
    void start(HINSTANCE hinst, const wchar_t *appname)
    {
        Input::GameInput::start();
        main_window = win32::create_window(
            hinst, 
			appname, 
			L"luieditor", 
			DefaultAppWidth, 
			DefaultAppHeight, 
			win32_message_callback, 
            ( Files::get_resource_root()/"icon_2.ico" ).wstring().c_str()
			//AppIcon
        );
        auto rect = main_window.client_rect;

        scene = new Scene();  // NO
        scene->add_lots_of_quads();

        // NO again
        rhi = new Renderer(main_window.hwnd, float(rect.right - rect.left), float(rect.bottom - rect.top)); 
        // DEBUG SELECTION RECT
        //rhi->add_selection_rect(-50.f, 50.f, -50.f, 50.f);
        rhi->create_world_grid_horizon();

        // this forces WM_SIZE, so that ImGui and the rendercontext have the correct size of the window
        SetWindowPos(
            main_window.hwnd, nullptr, 0, 0,
            (rect.right - rect.left + 1), // the + 1 is because the WM_SIZE message doesn't go through if the size is the same
            (rect.bottom - rect.top),
            SWP_NOMOVE
        );
    }

    void alt_draw_scene() {
        ZoneScoped("");
        std::vector<VertexPosColorTexcoord> upload_quads{scene->num_quads * 4};
        std::vector<int> upload_indices{};
        upload_indices.resize(scene->num_quads * 6);

        for(int i = 0; i < scene->num_quads; ++i) {
            XMFLOAT4 pos = scene->bounding_boxs[i];
            XMFLOAT4 color = scene->colors[i]; 
            
            VertexPosColorTexcoord quad_verts[] = {
                {
                    DirectX::XMFLOAT3{pos.x, pos.z, 0.f},
                    color,
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                },   
                {
                    DirectX::XMFLOAT3{pos.y, pos.z, 0.f},
                    color,
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                },
                {
                    DirectX::XMFLOAT3{pos.x, pos.w, 0.f},
                    color,
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                }, 
                {
                    DirectX::XMFLOAT3{pos.y, pos.w, 0.f},
                    color,
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

    }

    // TODO this should not be here 
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
                    DirectX::XMFLOAT4{0.9f, 0.9f, 0.9f, 1.0f},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                },   
                {
                    DirectX::XMFLOAT3{q.right, q.top, 0.0f},
                    DirectX::XMFLOAT4{0.9f, 0.9f, 0.9f, 1.0f},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                },
                {
                    DirectX::XMFLOAT3{q.left, q.bottom, 0.0f},
                    DirectX::XMFLOAT4{0.9f, 0.9f, 0.9f, 1.0f},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                }, 
                {
                    DirectX::XMFLOAT3{q.right, q.bottom, 0.0f},
                    DirectX::XMFLOAT4{0.9f, 0.9f, 0.9f, 1.0f},
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
        //ImGui::Text("num_quads: %d", scene->quadCount);
    }


    void update(float timestep)
    {
        static const char* sl_FrameTick = "update";
        FrameMarkStart(sl_FrameTick);

        rhi->set_and_clear_backbuffer();
        rhi->imgui_frame_begin();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        //if(!(ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard)) {
        //    Input::Ui::process_input_for_frame();
        //}

        Input::GameInput::update();
        Input::GameInput::draw_input_debug();
//        Input::Ui::debug_ui_input();

#ifdef DEBUG_IMGUI_WINDOW
        static bool show = false;
        ImGui::ShowDemoWindow(&show);
#endif

        static bool camera_mode = false;

        if(Input::GameInput::key_oneshot(VK_F1)) {
            camera_mode = !camera_mode;
        }

        if(camera_mode) {
            camera_controller::flycam_fps(timestep, rhi->camera);
        } else {
            camera_controller::dollycam(timestep, rhi->camera);
        }

        scene->update(timestep, rhi->camera);

        //draw_scene(timestep);
        alt_draw_scene();

//        rhi->draw_selection_rect();
//        rhi->draw_debug_lines();

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
                Input::Ui::parse_mouse(hwnd, msg, wparam, lparam); 
                break;
            }
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                Input::Ui::parse_keyboard(hwnd, msg, wparam, lparam);
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
                    scene->width = newWidth;
                    scene->height = newHeight;
                }
                break;
            }

            case WM_ACTIVATE:
            {
                Input::focus_changed(hwnd, msg, wparam, lparam);
                break;
            }
        }
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    void message_loop()
    {
        MSG msg;
        bool shouldClose = false;
        while (!shouldClose)
        {
            timer.tick();
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
            update(timer.elapsed_ms());
        }
        // this is where the program exits: 
        // TODO: call shutdown and free memory
        Input::GameInput::shutdown();
    }
};
