#include "Application.h"
#include "../Engine/files.h"
#include "../Engine/imgui_fmt.h"
#include "../Engine/logging.h"
#include <DirectXMath.h>
#include <Tracy.hpp>
#include <Windows.h>
#include <imgui.h>
#include <winuser.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

void app_create(AppState* app, HINSTANCE hinst, std::string appname) {

  app->window = win32::create_window(hinst,
                                     appname.c_str(),
                                     "luieditor",
                                     DefaultAppWidth,
                                     DefaultAppHeight,
                                     win32_message_callback,
                                     (Files::get_resource_root() / "icon_2.ico").string().c_str(),
                                     (void*)app);

}

void app_message_loop(AppState* app) {
  MSG msg;
  bool shouldClose = false;
  while (!shouldClose) {
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
      if (msg.message == WM_QUIT) {
        shouldClose = true;
        break;
      }
    }

    //app_update_hooks_run(app);
    app_systems_run(app);
  }
}

void app_begin_frame(AppState* app) {
  app->gfx->set_and_clear_backbuffer();
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
}

void app_end_frame(AppState* app) {
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  app->gfx->present();
}
 
void app_cbuffer_update(AppState* app) {
  /* TODO rename */
  //app->gfx->constant_buffer_map();
  /* TODO push constant buffers */
  //app->gfx->constant_buffer_unmap();
}

void app_systems_run(AppState* app) {
  
  input_update(app->input_system);
  app_begin_frame(app);

  f32 time_delta = 0.f;
  camera_input(app->camera_system, app->input_system, time_delta);


  app_cbuffer_update(app);

  /* run UI stuff */
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
  editor_main_ui(app);

  app_end_frame(app);

}

void app_shutdown(AppState* app) {

  delete app->gfx;
  delete app->debug_gfx;
  delete app->font_system;
  delete app->scene;
  delete app->input_system;
  delete app->camera_system;
  delete app->ray;
 // delete app->layout_system ;
 // delete app->live_game ;
}

void app_handle_window_create(AppState* app, 
                              HWND hwnd, 
                              i32 x, 
                              i32 y, 
                              i32 width, 
                              i32 height) {

  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)app);

  app->gfx = new Renderer(hwnd, width, height);
  app->debug_gfx = new DebugRenderSystem();
  app->font_system = new FontRenderer(app->gfx);
  app->scene = new Scene();
  app->input_system = new InputSystem();
  app->camera_system = new CameraSystem();
  app->ray = new RayCaster();
  app->layout_system = nullptr;
  app->live_game = nullptr;


  /* setup imgui */

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  LOG_INFO("imgui version: {} docking?: {}",
           ImGui::GetVersion(),
           (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) != 0);

  ImGui_ImplDX11_Init(app->gfx->device.Get(), app->gfx->context.Get());
  ImGui_ImplWin32_Init(hwnd);
}


void app_resize(AppState* app, i32 w, i32 h) {
  LOG_INFO("Resize to: {} {}", w, h);

  app->gfx->resize_swapchain_backbuffer(w, h, false);
}

void input_update(InputSystem* input) {
  input->proc_buffered_input();
  input->update();
}


void editor_main_ui(AppState* app) {
  editor_scene_display(app->scene);
  editor_camera_controls(app->camera_system);

  if(ImGui::Begin("Fonts")) {
    if(ImGui::Button("Add font")) {

      ImGui::Text("Not implemented yet");
      // TODO 
      // open file dialoge 
      // offload to a thread so rendering isnt blocked 

    }

  }
  ImGui::End();
}

void editor_scene_display(Scene* scene) {
  if(ImGui::Begin("Scene Tree")) {
    if(ImGui::Button("Add UI Element")) {
      scene->add_quad(Float4(-50, 50, -100, 100), Float4(1, 1, 1, 1), Float4());
    }

    for(int i = 0; i < scene->num_quads; ++i) {
      Float4 bb = scene->root_data.bounding_boxs[i];
      ImGui::TextFmt("box: {} {} {} {}", bb.x, bb.y, bb.z, bb.w );
    }

  }
  ImGui::End();
}

void editor_camera_controls(CameraSystem* camera_system) {
  if(ImGui::Begin("Camera settings")) {
    ImGui::InputFloat("Movement speed", &camera_system->normal_speed);
    ImGui::InputFloat("Fast speed", &camera_system->fast_speed);
    ImGui::InputFloat("Slow speed", &camera_system->slow_speed);
    ImGui::InputFloat("Smoothness", &camera_system->smoothness);
    ImGui::InputFloat("Inertia", &camera_system->inertia);
  }
  ImGui::End();
}

void camera_input(CameraSystem* camera_sys, InputSystem* input, f32 time_delta) {
  if(input->imgui_active)
    return;

  Camera* active_camera = camera_sys->get_active();

  if(active_camera->mode == CAM_FLYCAM) {
    camera_flycam(camera_sys, input, time_delta);
  } else if(active_camera->mode == CAM_DOLLY) {
    camera_dolly(camera_sys, input, time_delta);
  }
}



LRESULT CALLBACK win32_message_callback(HWND hwnd, 
                                        u32 msg,
                                        WPARAM wparam,
                                        LPARAM lparam) {

  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
    return true;

  if(msg == WM_CREATE) {
    CREATESTRUCT* create_info = (CREATESTRUCT*)lparam;
    AppState* app_ptr = (AppState*)create_info->lpCreateParams;
    app_handle_window_create(app_ptr, 
                             hwnd, 
                             create_info->x, 
                             create_info->y, 
                             create_info->cx, 
                             create_info->cy);

  }

  AppState* app = (AppState*)GetWindowLongPtr(hwnd, GWLP_USERDATA); 

  if (!app) {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }


  if (msg == WM_INPUT) {
    return app->input_system->raw_input(hwnd, msg, GET_RAWINPUT_CODE_WPARAM(wparam), (HRAWINPUT)lparam);
  }

  if (msg == WM_DESTROY) {
    PostQuitMessage(0);
    return 0;
  }

  if(msg == WM_SIZE) {
    u16 new_width = LOWORD(lparam);
    u16 new_height = HIWORD(lparam);
    app_resize(app, new_width, new_height);
  }

  app->input_system->handle_win32_input(hwnd, msg, wparam, lparam);

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

#define DEBUG_IMGUI_WINDOW 1

//namespace App {
//CameraSystem camera_system;
//DebugRenderSystem debug_render_system;
//FontRenderer *fonts;
//
//Texture2D test_texture{};
//
//void start(HINSTANCE hinst, const char *appname) {
//  logging_start();
//  LOG_INFO("starting app");
//
//  main_window = win32::create_window(hinst,
//                                     appname,
//                                     "luieditor",
//                                     DefaultAppWidth,
//                                     DefaultAppHeight,
//                                     win32_message_callback,
//                                     (Files::get_resource_root() / "icon_2.ico").string().c_str()
//                                     // AppIcon
//  );
//
//  RECT rect = main_window.client_rect;
//
//  // scene.add_quad(XMFLOAT4{0, 1280, 0, 720}, colors[Red], XMFLOAT4{});
//  scene.add_lots_of_quads();
//
//  // this forces WM_SIZE to be sent
//  // size of the win
//  // the + 1 is because the WM_SIZE message doesn't go through
//  // if the size is the samedow
//  SetWindowPos(main_window.hwnd, nullptr, 0, 0, (rect.right - rect.left + 1), (rect.bottom - rect.top), SWP_NOMOVE);
//}
//
//
//void update(float timestep) {
//  static const char *sl_FrameTick = "update";
//  static ImVec2 old_size{};
//  static ImVec2 new_size{};
//  static ViewportRegion scene_viewport{};
//
//  FrameMarkStart(sl_FrameTick);
//
//  renderer.imgui_frame_begin();
//  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
//
//  /* TODO: singleton is not a good idea here, it would be better 
//   * if we can just pass a pointer to systems interested in inputs */
//  u32 msg_count = InputSystem::instance().proc_buffered_input();
//  InputSystem &is = InputSystem::instance();
//  //is.imgui_active = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
//  //is.imgui_active = false;
//  is.update();
//
//  if (old_size.x != new_size.x || old_size.y != new_size.y) {
//    LOG_INFO("Resizing the render texture");
//    renderer.resize_render_texture(fmax(1.0f, new_size.x), fmax(1.0f, new_size.y));
//    renderer.update_shader_constants([&](PerSceneConsts& consts) {
//        consts.viewportSize.x = new_size.x;
//        consts.viewportSize.y = new_size.y;
//        consts.viewportSize.z = renderer.width;
//        consts.viewportSize.w = renderer.height;
//    });
//    old_size = new_size;
//  }
//
//  static char path_buffer[MAX_PATH]{};
//
//  static std::string lorem_ipsom = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor\n"
//    "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
//    "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum\n "
//    "dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt\n"
//    "mollit anim id est laborum";
//
//  static std::string alphabet = "hello there, This is some capital LEtterS!";
//
//  static FontID font_id = 0;
//  if (ImGui::Begin("Font atlases")) {
//    ImGui::InputText("font path:", path_buffer, MAX_PATH);
//    ImGui::SameLine();
//    if (ImGui::Button("+load")) {
//      font_id = fonts->load_font(&renderer, std::string(path_buffer), 24, 128, true);
//      memset(path_buffer, 0, MAX_PATH);
//    }
//    Texture2D *atlas = fonts->get_atlas();
//    ImGui::Image(atlas->srv.Get(), ImVec2(atlas->width, atlas->height));
//  }
//  ImGui::End();
//
//  /* Begin rendering to scene texture */
//  renderer.set_and_clear_render_texture();
//  renderer.draw_fullscreen_quad();
//  renderer.set_viewport(scene_viewport);
//  RayCaster::instance().set_viewport(scene_viewport);
//
//  camera_system.update(renderer, timestep);
//  camera_system.get_active().set_aspect_ratio(scene_viewport.w / scene_viewport.h);
//
//  // TODO camera system should handle this, as it can better keep track
//  // of which cameras are in what mode, what viewport they are looking at,
//  // etc.
//  camera_controller::flycam_fps(timestep, camera_system.get_active());
//
//  scene.update(&renderer, timestep);
//  scene.draw(&renderer);
//
//  fonts->draw_string(&lorem_ipsom, Float2(-200, -100), Float4{1, 0.0, 0.0, 1.0}, 0); 
//  static f32 monotonic_time = 0.0f;
//  monotonic_time += 0.001;
//  Float2 text_pos(100.f* sin(monotonic_time ), 100.f * cos(monotonic_time));
//
//  fonts->draw_string(&alphabet, text_pos, Float4(1, 1, 1, 1), 0);
//
//  fonts->submit(&renderer);
//
//  // draw the world orientation lines
//  // todo move to a function
//  XMVECTOR origin = XMVECTORF32{0, 0, 0, 0};
//  float length = 1000.f;
//
//  DebugRenderSystem::instance().debug_line_vec4(XMVECTORF32{-length, 0, 0, 0}, 
//                                                XMVECTORF32{length, 0, 0, 0},
//                                                colors[DebugColors::Blue]);
//
//  DebugRenderSystem::instance().debug_line_vec4(XMVECTORF32{ 0, -length, 0, 0}, 
//                                                XMVECTORF32{ 0,  length, 0, 0},
//                                                colors[DebugColors::Red]);
//
//  DebugRenderSystem::instance().debug_line_vec4(XMVECTORF32{ 0, 0, -length, 0 }, 
//                                                XMVECTORF32{ 0, 0, length,  0 },
//                                                colors[DebugColors::Green]);
//  DebugRenderSystem::instance().update(renderer);
//  DebugRenderSystem::instance().draw(renderer);
//
//  // draw to main viewport 
//  renderer.set_and_clear_backbuffer(); 
//  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
//
//  if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoBackground)) {
//    new_size = ImGui::GetContentRegionAvail();
//    ImVec2 pos = ImGui::GetWindowContentRegionMin();
//    {
//      ImVec2 vMin = ImGui::GetWindowContentRegionMin();
//      ImVec2 vMax = ImGui::GetWindowContentRegionMax();
//
//      vMin.x += ImGui::GetWindowPos().x;
//      vMin.y += ImGui::GetWindowPos().y;
//      vMax.x += ImGui::GetWindowPos().x;
//      vMax.y += ImGui::GetWindowPos().y;
//
//      scene_viewport.x = vMin.x;
//      scene_viewport.y = vMin.y;
//
//      // ImGui::GetForegroundDrawList()->AddRect(vMin, vMax, IM_COL32(50, 255, 128, 255));
//    }
//    scene_viewport.w = new_size.x;
//    scene_viewport.h = new_size.y;
//    //    LOG_INFO("viewport debug: {} {} {} {}",
//    //             scene_viewport.x,
//    //             scene_viewport.y,
//    //             scene_viewport.w,
//    //             scene_viewport.h);
//    is.imgui_active = !ImGui::IsWindowFocused() && ( ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard );
//    ImGui::Image((void *)renderer.render_texture.srv.Get(), old_size);
//  }
//  ImGui::End();
//  ImGui::PopStyleVar();
//
//#ifdef DEBUG_IMGUI_WINDOW
//  static bool show = false;
//  ImGui::ShowDemoWindow(&show);
//#endif
//
//  static bool camera_mode = false;
//
//#if 0
//  if (is.key_oneshot(VK_F1)) {
//    camera_mode = !camera_mode;
//  }
//#endif
//
//  // camera_system.update(renderer, timestep);
//  // camera_controller::flycam_fps(timestep, camera_system.get_active());
//  //// if (camera_mode) {
//  ////   camera_controller::flycam_fps(timestep, camera_system.get_active());
//  //// } else {
//  ////   camera_controller::dollycam(timestep, camera_system.get_active());
//  //// }
//
//  // scene.update(renderer, timestep);
//  // scene.draw(renderer);
//
//  ImDrawList *imdraw = ImGui::GetForegroundDrawList();
//
//  Vec4 screen_pos = RayCaster::instance().project(XMVECTORF32{0, 0, 0, 0});
//  imdraw->AddText(ImVec2(XMVectorGetX(screen_pos), XMVectorGetY(screen_pos)), ImU32(0xFFFFFFFF), "Origin");
//
//#if 0
//  if(Input::GameInput::key_oneshot(VK_F2)) { 
//
//    ray_cast::Ray pick_ray = ray_cast::screen_to_world_ray(Input::Ui::cursor().x, Input::Ui::cursor().y, renderer.width, renderer.height, camera_system.get_active(), XMMatrixIdentity());
//    DebugRenderSystem::instance().debug_ray(pick_ray);
//  }
//#endif
//
//
//  renderer.imgui_frame_end();
//  renderer.present();
//
//  FrameMarkEnd(sl_FrameTick);
//}
//
//LRESULT CALLBACK win32_message_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
//
//  if (msg == WM_INPUT) {
//    return InputSystem::instance().raw_input(hwnd, msg, GET_RAWINPUT_CODE_WPARAM(wparam), (HRAWINPUT)lparam);
//  }
//
//  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
//    return true;
//
//  InputSystem::instance().handle_win32_input(hwnd, msg, wparam, lparam);
//
//  switch (msg) {
//    case WM_DESTROY: {
//                       PostQuitMessage(0);
//                       return 0;
//                     }
//    case WM_CREATE:
//       renderer.init(hwnd, ((CREATESTRUCT *)lparam)->cx, ((CREATESTRUCT *)lparam)->cy);
//       scene.init(&renderer);
//       init_systems();
//                     break;
//
//    case WM_SIZE: {
//                    u16 nw = LOWORD(lparam);
//                    u16 newHeight = HIWORD(lparam);
//                    b8 wasMini = wparam == SIZE_MINIMIZED;
//                    renderer.resize_swapchain_backbuffer(nw, newHeight, wasMini);
//                    scene.width = nw;
//                    scene.height = newHeight;
//                  } break;
//    case WM_ACTIVATE: {
//                        // focus event
//                        break;
//                      }
//  }
//
//  return DefWindowProc(hwnd, msg, wparam, lparam);
//}
//
//void message_loop() {
//  MSG msg;
//  bool shouldClose = false;
//  while (!shouldClose) {
//    timer.tick();
//    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
//      TranslateMessage(&msg);
//      DispatchMessageW(&msg);
//      if (msg.message == WM_QUIT) {
//        shouldClose = true;
//        break;
//      }
//    }
//    update(timer.elapsed_ms());
//  }
//
//  shutdown_systems();
//  // this is where the program exits:
//  // TODO: call shutdown and free memory
//}
//
//void init_systems() {
//  InputSystem::instance().init(main_window.hwnd);
//  camera_system.init();
//  RayCaster::instance().init(&camera_system);
//  DebugRenderSystem::instance().init(renderer);
//  XMVECTOR a = XMVectorSet(0, 0, 0, 0);
//  XMVECTOR b = camera_system.get_active().origin;
//  DebugRenderSystem::instance().debug_line_vec4(a, b, colors[Red]);
//
//  fonts = new FontRenderer(&renderer);
//  fonts->load_font(&renderer, "C:/Windows/Fonts/CascadiaCode.ttf", 64, 128, true);
//  fonts->load_font(&renderer, "C:/Windows/Fonts/consola.ttf", 64, 128, true);
//
//  // FontAtlas *atlas;
//  // if ((atlas = fonts.get_ptr("Cascadia Code"))) {
//  //   create_params.initial_data = fonts.get_atlas_texture("Cascadia Code");
//  //   create_params.usage = D3D11_USAGE_DEFAULT;
//  //   create_params.format = DXGI_FORMAT_R8G8B8A8_UNORM;
//  //   create_params.desired_width = atlas->width;
//  //   create_params.desired_height = atlas->height;
//  //   if (renderer.create_texture(create_params, test_texture)) {
//  //     LOG_INFO("created texture for cascadia code atlas!");
//  //   } else {
//  //     LOG_WARNING("FAILED to create texture for cascadia code atlas!");
//  //   }
//  // }
//}
//
//void shutdown_systems() { 
//  InputSystem::instance().shutdown();
//}
//
//}; // namespace App
