// T7LuaEditor.cpp : Defines the entry point for the application.
//

#include "main.h"
#include "./Application/Application.h"

int WINAPI wWinMain(HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PWSTR pCmdLine,
                    int nCmdShow) {

  CoInitialize(nullptr);
//  App::start(hInstance, "Equinox");
//  App::message_loop();

  AppState state;
  app_create(&state, hInstance, "Equinox");
  app_message_loop(&state);
  app_shutdown(&state);



  return 0;
}
