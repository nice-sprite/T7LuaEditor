// T7LuaEditor.cpp : Defines the entry point for the application.
//

#include "main.h"
#include "./Application/Application.h"
//#include <memory>


void do_it() {


}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{

    CoInitialize(nullptr);
    app::start(hInstance, L"Priscilla");
    app::message_loop();
    return 0;
}
