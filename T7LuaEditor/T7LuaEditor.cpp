// T7LuaEditor.cpp : Defines the entry point for the application.
//

#include "T7LuaEditor.h"
#include <memory>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    CoInitialize(NULL);
    Window window(hInstance, L"Lua Editor", L"luaeditor", 1280, 720);
    window.ProcessMessages();
    return 0;
}
