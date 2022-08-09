#include "Application.h"
#include <Windows.h>
#include <imgui.h>
#include <Tracy.hpp> 
namespace app 
{
    void start(HINSTANCE hinst, const wchar_t *appname)
    {
        mainWindow = win32::create_window(hinst, appname, L"luaeditor", AppWidth, AppHeight, win32_message_callback);
        win32::set_window_icon(mainWindow.hwnd, AppIcon);
        auto rect = mainWindow.clientRect;

        scene = new Scene();  

        rhi = new Renderer(mainWindow.hwnd, float(rect.right - rect.left), float(rect.bottom - rect.top)); 

        //rhi->create_render_pass_resources("scene", scene->passDef);

        SetWindowPos(mainWindow.hwnd, nullptr, 0, 0,
                     (rect.right - rect.left + 1), // the + 1 is because the WM_SIZE message doesn't go through if the size is the same
                     (rect.bottom - rect.top),
                     SWP_NOMOVE);
    }

    void draw_scene(float ts)
    {


    }


    void tick(float timestep)
    {
        static const char* sl_FrameTick = "Tick";
        FrameMarkStart(sl_FrameTick);

        rhi->set_and_clear_backbuffer();
        rhi->imgui_frame_begin();
        static bool show = true;
        {
            ZoneScoped("ImGui::ShowDemoWindow()");
            ImGui::ShowDemoWindow(&show);

        }
        input::process_input_for_frame();
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
