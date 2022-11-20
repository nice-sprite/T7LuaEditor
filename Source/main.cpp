// T7LuaEditor.cpp : Defines the entry point for the application.
//

#include "main.h"
#include "./Application/Application.h"
//#include <memory>

int WINAPI wWinMain(HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PWSTR pCmdLine,
                    int nCmdShow) {

  CoInitialize(nullptr);
  App::start(hInstance, "Equinox");
  App::message_loop();
  return 0;
}
