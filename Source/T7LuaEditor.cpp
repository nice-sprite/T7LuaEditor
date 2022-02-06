// T7LuaEditor.cpp : Defines the entry point for the application.
//

#include "T7LuaEditor.h"
#include "./Application/Application.h"
#include <memory>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    CoInitialize(nullptr);
    Application::StartApplication(hInstance, L"Priscilla");
    Application::MessageLoop();
    return 0;
}
