#include "Application.h"
#include "../Engine/camera_controller.h"
#include "../Engine/files.h"
#include "../Engine/logging.h"
#include <DirectXMath.h>
#include <Tracy.hpp>
#include <Windows.h>
#include <imgui.h>
#include <winuser.h>

namespace App {

CameraSystem camera_system;
DebugRenderSystem debug_render_system;

void start(HINSTANCE hinst, const char *appname) {
  logging_start();
  LOG_INFO("starting app");

  main_window = win32::create_window(
      hinst,
      appname,
      "luieditor",
      DefaultAppWidth,
      DefaultAppHeight,
      win32_message_callback,
      (Files::get_resource_root() / "icon_2.ico").string().c_str()
      // AppIcon
  );
  RECT rect = main_window.client_rect;

  // scene.add_quad(XMFLOAT4{0, 1280, 0, 720}, colors[Red], XMFLOAT4{});
  scene.add_lots_of_quads();

  // this forces WM_SIZE to be sent
  // size of the win
  // the + 1 is because the WM_SIZE message doesn't go through
  // if the size is the samedow
  SetWindowPos(main_window.hwnd,
               nullptr,
               0,
               0,
               (rect.right - rect.left + 1),
               (rect.bottom - rect.top),
               SWP_NOMOVE);
}

void update(float timestep) {
  static const char *sl_FrameTick = "update";
  FrameMarkStart(sl_FrameTick);

  renderer.set_and_clear_backbuffer();
  renderer.imgui_frame_begin();
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                               ImGuiDockNodeFlags_PassthruCentralNode);

  u32 msg_count = InputSystem::instance().proc_buffered_input();
  // if (msg_count > 0) {
  //   LOG_INFO("read {} rawinput packets this frame", msg_count);
  // }

  // if(!(ImGui::GetIO().WantCaptureMouse ||
  // ImGui::GetIO().WantCaptureKeyboard)) {
  //     Input::Ui::process_input_for_frame();
  // }

  InputSystem &is = InputSystem::instance();
  is.imgui_active = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
  is.update();

#ifdef DEBUG_IMGUI_WINDOW
  static bool show = false;
  ImGui::ShowDemoWindow(&show);
#endif

  static bool camera_mode = false;

#if 0
  if (is.key_oneshot(VK_F1)) {
    camera_mode = !camera_mode;
  }
#endif

  camera_system.update(renderer, timestep);
  camera_controller::flycam_fps(timestep, camera_system.get_active());
  // if (camera_mode) {
  //   camera_controller::flycam_fps(timestep, camera_system.get_active());
  // } else {
  //   camera_controller::dollycam(timestep, camera_system.get_active());
  // }

  scene.update(renderer, timestep);
  scene.draw(renderer);

  ImDrawList *imdraw = ImGui::GetForegroundDrawList();

  Vec4 screen_pos = RayCaster::instance().project(XMVECTORF32{0, 0, 0, 0});
  imdraw->AddText(ImVec2(XMVectorGetX(screen_pos), XMVectorGetY(screen_pos)),
                  ImU32(0xFFFFFFFF),
                  "Origin");

#if 0
  if(Input::GameInput::key_oneshot(VK_F2)) { 

    ray_cast::Ray pick_ray = ray_cast::screen_to_world_ray(Input::Ui::cursor().x, Input::Ui::cursor().y, renderer.width, renderer.height, camera_system.get_active(), XMMatrixIdentity());
    DebugRenderSystem::instance().debug_ray(pick_ray);
  }
#endif

  DebugRenderSystem::instance().update(renderer);
  DebugRenderSystem::instance().draw(renderer);

  renderer.imgui_frame_end();
  renderer.present();

  FrameMarkEnd(sl_FrameTick);
}

LRESULT CALLBACK win32_message_callback(HWND hwnd,
                                        UINT msg,
                                        WPARAM wparam,
                                        LPARAM lparam) {

  if (msg == WM_INPUT) {
    return InputSystem::instance().raw_input(hwnd,
                                             msg,
                                             GET_RAWINPUT_CODE_WPARAM(wparam),
                                             (HRAWINPUT)lparam);
  }

  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
    return true;

  InputSystem::instance().handle_win32_input(hwnd, msg, wparam, lparam);

  switch (msg) {
  case WM_DESTROY: {
    PostQuitMessage(0);
    return 0;
  }
  case WM_CREATE:
    renderer.init(hwnd,
                  ((CREATESTRUCT *)lparam)->cx,
                  ((CREATESTRUCT *)lparam)->cy);
    scene.init(renderer);
    init_systems();
    break;

  case WM_SIZE: {
    u16 nw = LOWORD(lparam);
    u16 newHeight = HIWORD(lparam);
    b8 wasMini = wparam == SIZE_MINIMIZED;
    renderer.resize_swapchain_backbuffer(nw, newHeight, wasMini);
    scene.width = nw;
    scene.height = newHeight;
  } break;
  case WM_ACTIVATE: {
    // focus event
    break;
  }
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

void message_loop() {
  MSG msg;
  bool shouldClose = false;
  while (!shouldClose) {
    timer.tick();
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
      if (msg.message == WM_QUIT) {
        shouldClose = true;
        break;
      }
    }
    update(timer.elapsed_ms());
  }

  shutdown_systems();
  // this is where the program exits:
  // TODO: call shutdown and free memory
}

void init_systems() {
  InputSystem::instance().init(main_window.hwnd);
  camera_system.init(renderer);
  RayCaster::instance().init(&camera_system);
  DebugRenderSystem::instance().init(renderer);
  XMVECTOR a = XMVectorSet(0, 0, 0, 0);
  XMVECTOR b = camera_system.get_active().origin;
  DebugRenderSystem::instance().debug_line_vec4(a, b, colors[Red]);
}

void shutdown_systems() { InputSystem::instance().shutdown(); }
}; // namespace App
