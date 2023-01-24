#include "Application.h"
#include "../Engine/camera_controller.h"
#include "../Engine/files.h"
#include "../Engine/imgui_fmt.h"
#include "../Engine/logging.h"
#include <DirectXMath.h>
#include <Tracy.hpp>
#include <Windows.h>
#include <imgui.h>
#include <winuser.h>

#define DEBUG_IMGUI_WINDOW 1

namespace App {
CameraSystem camera_system;
DebugRenderSystem debug_render_system;
FontRenderer *fonts;

Texture2D test_texture{};

void start(HINSTANCE hinst, const char *appname) {
  logging_start();
  LOG_INFO("starting app");

  main_window = win32::create_window(hinst,
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
  SetWindowPos(main_window.hwnd, nullptr, 0, 0, (rect.right - rect.left + 1), (rect.bottom - rect.top), SWP_NOMOVE);
}

void update(float timestep) {
  static const char *sl_FrameTick = "update";
  static ImVec2 old_size{};
  static ImVec2 new_size{};
  static ViewportRegion scene_viewport{};

  FrameMarkStart(sl_FrameTick);

  renderer.imgui_frame_begin();
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

  u32 msg_count = InputSystem::instance().proc_buffered_input();
  InputSystem &is = InputSystem::instance();
  // is.imgui_active =
  //     ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
  is.imgui_active = false;
  is.update();

  if (old_size.x != new_size.x || old_size.y != new_size.y) {
    LOG_INFO("Resizing the render texture");
    renderer.resize_render_texture(fmax(1.0f, new_size.x), fmax(1.0f, new_size.y));
    renderer.update_shader_constants([&](PerSceneConsts& consts) {

        consts.viewportSize.x = new_size.x;
        consts.viewportSize.y = new_size.y;
        consts.viewportSize.z = renderer.width;
        consts.viewportSize.w = renderer.height;

        });
    old_size = new_size;
  }

  static char path_buffer[MAX_PATH]{};

  static std::string lorem_ipsom = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor\n"
    " incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
" nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum\n "
" dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt\n"
  "mollit anim id est laborum";

  static std::string alphabet = "int main() { \n    return \"a string\";\n}";
  static bool fuckon = true;
  for (int i = 0; i < 128 && !fuckon; ++i) {
    alphabet += (char)i;
  }
  fuckon = true;

  static FontID font_id = 0;
  if (ImGui::Begin("Font atlases")) {
    ImGui::InputText("font path:", path_buffer, MAX_PATH);
    ImGui::SameLine();
    if (ImGui::Button("+load")) {
      font_id = fonts->load_font(&renderer, std::string(path_buffer), 24, 128, true);
      memset(path_buffer, 0, MAX_PATH);
    }
    Texture2D *atlas = fonts->get_atlas();
    ImGui::Image(atlas->srv.Get(), ImVec2(atlas->width, atlas->height));
  }
  ImGui::End();

  //draw to render texture
  renderer.set_and_clear_render_texture();
  renderer.draw_fullscreen_quad();
  renderer.set_viewport(scene_viewport);
  RayCaster::instance().set_viewport(scene_viewport);

  camera_system.update(renderer, timestep);
  camera_system.get_active().set_aspect_ratio(scene_viewport.w / scene_viewport.h);

  // TODO camera system should handle this, as it can better keep track
  // of which cameras are in what mode, what viewport they are looking at,
  // etc
  camera_controller::flycam_fps(timestep, camera_system.get_active());

  scene.update(renderer, timestep);
  scene.draw(renderer);
  fonts->draw_string((char *)alphabet.c_str(), alphabet.length(), Float4{0, 0.0, 0.0, 1.0}, 1);
  //fonts->draw_string((char *)lorem_ipsom.c_str(), lorem_ipsom.length(), Float4{1, 1, 1, 1});
  fonts->submit(&renderer);

  // draw the world orientation lines
  // todo move to a function
  XMVECTOR origin = XMVECTORF32{0, 0, 0, 0};
  float length = 1000.f;
  DebugRenderSystem::instance().debug_line_vec4(XMVECTORF32{-length, 0, 0, 0}, 
                                                XMVECTORF32{length, 0, 0, 0},
                                                colors[DebugColors::Blue]);
  DebugRenderSystem::instance().debug_line_vec4(XMVECTORF32{ 0, -length, 0, 0}, 
                                                XMVECTORF32{ 0,  length, 0, 0},
                                                colors[DebugColors::Red]);

  DebugRenderSystem::instance().debug_line_vec4(XMVECTORF32{ 0, 0, -length, 0 }, 
                                                XMVECTORF32{ 0, 0, length,  0 },
                                                colors[DebugColors::Green]);
  DebugRenderSystem::instance().update(renderer);
  DebugRenderSystem::instance().draw(renderer);

  // draw to main viewport 
  renderer.set_and_clear_backbuffer(); 
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoBackground)) {
    new_size = ImGui::GetContentRegionAvail();
    ImVec2 pos = ImGui::GetWindowContentRegionMin();
    {
      ImVec2 vMin = ImGui::GetWindowContentRegionMin();
      ImVec2 vMax = ImGui::GetWindowContentRegionMax();

      vMin.x += ImGui::GetWindowPos().x;
      vMin.y += ImGui::GetWindowPos().y;
      vMax.x += ImGui::GetWindowPos().x;
      vMax.y += ImGui::GetWindowPos().y;

      scene_viewport.x = vMin.x;
      scene_viewport.y = vMin.y;

      // ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(50, 255, 128, 255));
    }
    scene_viewport.w = new_size.x;
    scene_viewport.h = new_size.y;
    //    LOG_INFO("viewport debug: {} {} {} {}",
    //             scene_viewport.x,
    //             scene_viewport.y,
    //             scene_viewport.w,
    //             scene_viewport.h);
    ImGui::Image((void *)renderer.render_texture.srv.Get(), old_size);
  }
  ImGui::End();
  ImGui::PopStyleVar();

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

  // camera_system.update(renderer, timestep);
  // camera_controller::flycam_fps(timestep, camera_system.get_active());
  //// if (camera_mode) {
  ////   camera_controller::flycam_fps(timestep, camera_system.get_active());
  //// } else {
  ////   camera_controller::dollycam(timestep, camera_system.get_active());
  //// }

  // scene.update(renderer, timestep);
  // scene.draw(renderer);

  ImDrawList *imdraw = ImGui::GetForegroundDrawList();

  Vec4 screen_pos = RayCaster::instance().project(XMVECTORF32{0, 0, 0, 0});
  imdraw->AddText(ImVec2(XMVectorGetX(screen_pos), XMVectorGetY(screen_pos)), ImU32(0xFFFFFFFF), "Origin");

#if 0
  if(Input::GameInput::key_oneshot(VK_F2)) { 

    ray_cast::Ray pick_ray = ray_cast::screen_to_world_ray(Input::Ui::cursor().x, Input::Ui::cursor().y, renderer.width, renderer.height, camera_system.get_active(), XMMatrixIdentity());
    DebugRenderSystem::instance().debug_ray(pick_ray);
  }
#endif


  renderer.imgui_frame_end();
  renderer.present();

  FrameMarkEnd(sl_FrameTick);
}

LRESULT CALLBACK win32_message_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

  if (msg == WM_INPUT) {
    return InputSystem::instance().raw_input(hwnd, msg, GET_RAWINPUT_CODE_WPARAM(wparam), (HRAWINPUT)lparam);
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
    renderer.init(hwnd, ((CREATESTRUCT *)lparam)->cx, ((CREATESTRUCT *)lparam)->cy);
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
  camera_system.init();
  RayCaster::instance().init(&camera_system);
  DebugRenderSystem::instance().init(renderer);
  XMVECTOR a = XMVectorSet(0, 0, 0, 0);
  XMVECTOR b = camera_system.get_active().origin;
  DebugRenderSystem::instance().debug_line_vec4(a, b, colors[Red]);

  fonts = new FontRenderer(&renderer);
  fonts->load_font(&renderer, "C:/Windows/Fonts/CascadiaCode.ttf", 24, 128, true);
  fonts->load_font(&renderer, "C:/Windows/Fonts/consola.ttf", 24, 128, true);

  // FontAtlas *atlas;
  // if ((atlas = fonts.get_ptr("Cascadia Code"))) {
  //   create_params.initial_data = fonts.get_atlas_texture("Cascadia Code");
  //   create_params.usage = D3D11_USAGE_DEFAULT;
  //   create_params.format = DXGI_FORMAT_R8G8B8A8_UNORM;
  //   create_params.desired_width = atlas->width;
  //   create_params.desired_height = atlas->height;
  //   if (renderer.create_texture(create_params, test_texture)) {
  //     LOG_INFO("created texture for cascadia code atlas!");
  //   } else {
  //     LOG_WARNING("FAILED to create texture for cascadia code atlas!");
  //   }
  // }
}

void shutdown_systems() { InputSystem::instance().shutdown(); }
}; // namespace App
