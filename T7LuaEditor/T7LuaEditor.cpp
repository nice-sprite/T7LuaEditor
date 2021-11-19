// T7LuaEditor.cpp : Defines the entry point for the application.
//

#include "T7LuaEditor.h"
#include <memory>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    auto hwnd = WindowUtil::NewWindow(L"Lua editor", hInstance, nCmdShow);

    Gfx::StartDx11(hwnd);
    Gfx::PrepareImGui();
    // main loop
    MSG msg;
    bool shouldClose = false;
    while (!shouldClose)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) { 
                shouldClose = true;  
                break; 
            }
        }
        Gfx::Render();
    }
    WindowUtil::Cleanup();
    return 0;
}
