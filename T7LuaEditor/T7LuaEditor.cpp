// T7LuaEditor.cpp : Defines the entry point for the application.
//

#include "T7LuaEditor.h"
#include <memory>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    CoInitialize(nullptr);
    Window window(hInstance, L"Lua Editor", L"luaeditor", 1920, 1080);
    window.ProcessMessages();
    return 0;
}
