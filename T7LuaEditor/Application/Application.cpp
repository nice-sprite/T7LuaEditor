
#include "Application.h"
#include <Windows.h>

namespace Application
{
    void StartApplication(HINSTANCE hinst, const wchar_t *appname)
    {
        mainWindow = WindowUtil::Create(hinst, appname, L"luaeditor", AppWidth, AppHeight, WndMsgCallback);
        WindowUtil::SetIcon(mainWindow.hwnd, AppIcon);
        auto rect = mainWindow.clientRect;
        rhi = std::make_unique<Renderer>(
            mainWindow.hwnd,
            float(rect.right - rect.left),
            float(rect.bottom - rect.top));
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui_ImplDX11_Init(rhi->GetDevice(), rhi->GetContext());
        ImGui_ImplWin32_Init(mainWindow.hwnd);
        scene = std::make_unique<Scene2D>(rhi.get());
        SetWindowPos(mainWindow.hwnd, nullptr, 0, 0,
                     (rect.right - rect.left + 1), // the + 1 is because the WM_SIZE message doesn't go through if the size is the same
                     (rect.bottom - rect.top),
                     SWP_NOMOVE);
    }

    void Tick(float timestep)
    {

        if (rhi)
        {
            rhi->ClearRTV();
            ImGuiBeginFrame();
            scene->HandleInput(mouse, keyboard);
            scene->RenderScene(rhi.get(), timestep);
            scene->HandleUI();
            ImGuiEndFrame();
            rhi->Present();
        }
    }

    LRESULT CALLBACK WndMsgCallback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
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
        {
            int x, y;
            x = (int)(short)LOWORD(lparam);
            y = (int)(short)HIWORD(lparam);
            int wheelDelta = GET_WHEEL_DELTA_WPARAM(wparam);
            mouse.UpdateWheelDelta(wheelDelta);
            break;
        }
        case WM_MOUSEMOVE:
        {
            int x, y;
            x = (int)(short)LOWORD(lparam);
            y = (int)(short)HIWORD(lparam);
            mouse.UpdatePos(x, y);
            break;
            // update mouse x and y
        }
        case WM_LBUTTONDOWN:
        {
            int x, y;
            x = (int)(short)LOWORD(lparam);
            y = (int)(short)HIWORD(lparam);
            mouse.buttons.leftButton = true;
            mouse.UpdatePos(x, y);
            break;
            // lbutton down
        }
        case WM_RBUTTONDOWN:
        {
            // lbutton down
            int x, y;
            x = (int)(short)LOWORD(lparam);
            y = (int)(short)HIWORD(lparam);
            mouse.buttons.rightButton = true;
            mouse.UpdatePos(x, y);
            break;
        }
        case WM_LBUTTONUP:
        {
            // lbutton up
            int x, y;
            x = (int)(short)LOWORD(lparam);
            y = (int)(short)HIWORD(lparam);
            mouse.buttons.leftButton = false;
            mouse.UpdatePos(x, y);
            break;
        }
        case WM_RBUTTONUP:
        {
            // rbutton up
            int x, y;
            x = (int)(short)LOWORD(lparam);
            y = (int)(short)HIWORD(lparam);
            mouse.buttons.rightButton = false;
            mouse.UpdatePos(x, y);
            break;
        }
        case WM_KEYDOWN:
        {
            // update keyboard
        }
        case WM_SIZE:
        {
            if (rhi)
            {

                rhi->Resize(lparam, wparam);
                scene->Resize(lparam, wparam);
            }
        }
        }

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    void MessageLoop()
    {
        frameTimer.Start();
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
            Tick(frameTimer.StopMS());
        }
    }

    void ImGuiBeginFrame()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiEndFrame()
    {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

};
