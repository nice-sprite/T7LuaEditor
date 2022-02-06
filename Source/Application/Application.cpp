
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
            Input::ProcessInput();
            scene->RenderScene(rhi.get(), timestep);
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
            case WM_MOUSEMOVE:
            case WM_RBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            {
                Input::CacheMouseEvents(hwnd, msg, wparam, lparam); // pushes events
                break;
            }
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                Input::CacheKeyboardEvents(hwnd, msg, wparam, lparam);
                break;
            }
            case WM_SIZE:
            {
                if (rhi)
                {
                    rhi->Resize(lparam, wparam);
                    scene->Resize(lparam, wparam); 
                }
                break;
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
