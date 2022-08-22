#include "Application.h"
#include <Windows.h>
#include <imgui.h>
#include <Tracy.hpp> 
namespace app 
{
    void start(HINSTANCE hinst, const wchar_t *appname)
    {
        mainWindow = win32::create_window(hinst, appname, L"luaeditor", AppWidth, AppHeight, win32_message_callback, AppIcon);
        auto rect = mainWindow.clientRect;

        scene = new Scene();  

        rhi = new Renderer(mainWindow.hwnd, float(rect.right - rect.left), float(rect.bottom - rect.top)); 

        SetWindowPos(mainWindow.hwnd, nullptr, 0, 0,
                     (rect.right - rect.left + 1), // the + 1 is because the WM_SIZE message doesn't go through if the size is the same
                     (rect.bottom - rect.top),
                     SWP_NOMOVE);

        /*input::register_mouse_click_callback([&](input::mouse_t& mouse, float x, float y, WPARAM extra) ->bool  {
            if(input::Btn_Left(mouse.stateFlags)){
                rhi->add_debug_line(XMFLOAT3(100.0, 50.0, 0.0), XMFLOAT3(0.0, 0.0, 800.0), XMFLOAT4(1.0, 1.0, 1.0, 1.0) );
            }
            return true;
        });*/
    }

    void draw_scene(float ts) {
        ZoneScoped("");
        std::vector<VertexPosColorTexcoord> upload_quads{scene->quadCount * 4};
        std::vector<int> upload_indices{};
        upload_indices.resize(scene->quadCount * 6);
        for(int i = 0; i < scene->quadCount; ++i) {
            auto q = scene->quads[i];

            VertexPosColorTexcoord quad_verts[] = {
                {
                    DirectX::XMFLOAT3{q.left, q.top, 0.0f},
                    DirectX::XMFLOAT4{1.0, 0.0, 0.0, 0.5},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                },   
                {
                    DirectX::XMFLOAT3{q.right, q.top, 0.0f},
                    DirectX::XMFLOAT4{0.0, 1.0, 0.0, 0.5},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                },
                {
                    DirectX::XMFLOAT3{q.left, q.bottom, 0.0f},
                    DirectX::XMFLOAT4{0.0, 0.0, 1.0, 0.5},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                }, 
                {
                    DirectX::XMFLOAT3{q.right, q.bottom, 0.0f},
                    DirectX::XMFLOAT4{1.0, 1.0, 1.0, 1.0},
                    DirectX::XMFLOAT2{0.0f, 0.0f}
                },   
            };

            upload_quads[i * 4 + 0] = quad_verts[0];
            upload_quads[i * 4 + 1] = quad_verts[1];
            upload_quads[i * 4 + 2] = quad_verts[2];
            upload_quads[i * 4 + 3] = quad_verts[3];

            /* 
           upload_indices[i * 6 + 0] = i * 4 + 0;
           upload_indices[i * 6 + 1] = i * 4 + 1;
           upload_indices[i * 6 + 2] = i * 4 + 2;
           upload_indices[i * 6 + 3] = i * 4 + 1;
           upload_indices[i * 6 + 4] = i * 4 + 3;
           upload_indices[i * 6 + 5] = i * 4 + 2;
           */ 

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
        input::process_input_for_frame();

        static bool show = true;
        ImGui::ShowDemoWindow(&show);
        

        draw_scene(timestep);
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
            case WM_RBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            {
                input::cache_mouse_input_for_frame(hwnd, msg, wparam, lparam); // pushes events
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
    }
    // this is where the program exits: 
    // TODO: call shutdown and free memory


};
