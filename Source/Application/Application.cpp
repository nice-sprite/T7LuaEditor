#include "Application.h"
#include "../Engine/camera_controller.h"
#include "../Engine/files.h"
#include "../Engine/logging.h"
#include <DirectXMath.h>
#include <Tracy.hpp>
#include <Windows.h>
#include <imgui.h>

namespace App {
  CameraSystem camera_system;
  DebugRenderSystem debug_render_system;
void start(HINSTANCE hinst, const char *appname) {
  logging_start();
  LOG_INFO("starting app");

  main_window = win32::create_window(
      hinst, appname, "luieditor", DefaultAppWidth, DefaultAppHeight,
      win32_message_callback,
      (Files::get_resource_root() / "icon_2.ico").string().c_str()
      // AppIcon
  );
  auto rect = main_window.client_rect;

  //scene.add_quad(XMFLOAT4{0, 1280, 0, 720}, colors[Red], XMFLOAT4{});
  scene.add_lots_of_quads();

  // this forces WM_SIZE to be sent
  // size of the window
  SetWindowPos(main_window.hwnd, nullptr, 0, 0,
               (rect.right - rect.left +
                1), // the + 1 is because the WM_SIZE message doesn't go through
                    // if the size is the same
               (rect.bottom - rect.top), SWP_NOMOVE);
}

void update(float timestep) {
  static const char *sl_FrameTick = "update";
  FrameMarkStart(sl_FrameTick);

  renderer.set_and_clear_backbuffer();
  renderer.imgui_frame_begin();
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                               ImGuiDockNodeFlags_PassthruCentralNode);

  // if(!(ImGui::GetIO().WantCaptureMouse ||
  // ImGui::GetIO().WantCaptureKeyboard)) {
  //     Input::Ui::process_input_for_frame();
  // }

  Input::GameInput::update();
  Input::GameInput::draw_input_debug();
  Input::Ui::debug_ui_input();

#ifdef DEBUG_IMGUI_WINDOW
  static bool show = false;
  ImGui::ShowDemoWindow(&show);
#endif

  static bool camera_mode = false;

  if (Input::GameInput::key_oneshot(VK_F1)) {
    camera_mode = !camera_mode;
  }

  camera_system.update(renderer, timestep);
  if (camera_mode) {
    camera_controller::flycam_fps(timestep, camera_system.get_active());
  } else {
    camera_controller::dollycam(timestep, camera_system.get_active());
  }

  scene.update(renderer, timestep, camera_system.get_active());
  scene.draw(renderer);

  auto imdraw = ImGui::GetForegroundDrawList();
  XMFLOAT2 origin = ray_cast::world_to_screen(XMFLOAT3(0, 0, 0), renderer.width, renderer.height, camera_system.get_active());
  imdraw->AddText(ImVec2(origin.x, origin.y), ImU32(0xFFFFFFFF), "Origin");
  if(Input::GameInput::key_oneshot(VK_F2)) { 

    ray_cast::Ray pick_ray = ray_cast::screen_to_world_ray(Input::Ui::cursor().x, Input::Ui::cursor().y, renderer.width, renderer.height, camera_system.get_active(), XMMatrixIdentity());
    DebugRenderSystem::instance().debug_ray(pick_ray);
  } 

  DebugRenderSystem::instance().update(renderer);
  DebugRenderSystem::instance().draw(renderer);

  renderer.imgui_frame_end();
  renderer.present();

  FrameMarkEnd(sl_FrameTick);
}

LRESULT CALLBACK win32_message_callback(HWND hwnd, UINT msg, WPARAM wparam,
                                        LPARAM lparam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
    return true;
  switch (msg) {
  case WM_DESTROY: {
    PostQuitMessage(0);
    return 0;
  }
  case WM_CREATE:
    renderer.init(hwnd, ((CREATESTRUCT *)lparam)->cx,
                  ((CREATESTRUCT *)lparam)->cy);
    scene.init(renderer);
    init_systems();
    break;

  case WM_MOUSEWHEEL:
  case WM_MOUSEMOVE:

  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_RBUTTONDBLCLK:

  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_LBUTTONDBLCLK:

  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_MBUTTONDBLCLK:

  case WM_XBUTTONUP:
  case WM_XBUTTONDOWN:
  case WM_XBUTTONDBLCLK: {
    Input::Ui::parse_mouse(hwnd, msg, wparam, lparam);
    break;
  }
  case WM_KEYDOWN:
  case WM_KEYUP: {
    Input::Ui::parse_keyboard(hwnd, msg, wparam, lparam);
    break;
  }
  case WM_SIZE: {
    auto nw = LOWORD(lparam);
    auto newHeight = HIWORD(lparam);
    auto wasMini = wparam == SIZE_MINIMIZED;
    renderer.resize_swapchain_backbuffer(nw, newHeight, wasMini);
    scene.width = nw;
    scene.height = newHeight;
  } break;
  case WM_ACTIVATE: {
    Input::focus_changed(hwnd, msg, wparam, lparam);
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
  // this is where the program exits:
  // TODO: call shutdown and free memory
  Input::GameInput::shutdown();
}

void init_systems() {
  camera_system.init(renderer);
  DebugRenderSystem::instance().init(renderer);
  XMVECTOR a = XMVectorSet(0, 0, 0, 0);
  XMVECTOR b = camera_system.get_active().origin;
  DebugRenderSystem::instance().debug_line_vec4(a, b, colors[Red]);
  Input::GameInput::start();
}
}; // namespace App
