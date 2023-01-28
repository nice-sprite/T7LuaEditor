#include "../Engine/camera_system.h"
#include "../Engine/debug_render.h"
#include "../Engine/font_renderer.h"
#include "../Engine/input_system.h"
#include "../Engine/renderer.h"
#include "../Engine/scene.h"
#include "../Engine/timer.h"
#include "../Engine/win32_lib.h"

static constexpr auto DefaultAppWidth = 1920;
static constexpr auto DefaultAppHeight = 1080;
static constexpr auto AppTitle = "equinox - by nice_sprite";


LRESULT CALLBACK win32_message_callback(HWND hwnd, 
                                        u32 msg,
                                        WPARAM wparam,
                                        LPARAM lparam);


using PreframeUpdateFn = void (*)(struct AppState* app, f64 frame_time);

struct AppState {
  std::string name;
  Float2 viewport_dimensions;

  win32::Window window;
  Timer frame_timer;
  u64 frame_counter;

  Renderer* gfx;
  DebugRenderSystem* debug_gfx;
  FontRenderer* font_system;
  Scene* scene;
  CameraSystem* camera_system;
  InputSystem* input_system;
  RayCaster* ray;
  struct UILayoutSystem* layout_system;
  struct RemoteLUIUpdateSystem* live_game;

  PreframeUpdateFn preframe_fns[32]{}; // can put debug/metric capture shit here, conditional input fns
};

void app_create(AppState* app, HINSTANCE hinst, std::string appname);

/* Handles creating the renderer and renderer-dependent systems
 * This is because a window is required to be created for directX to work  
 * */ 
void app_handle_window_create(AppState* app, 
                              HWND hwnd, 
                              i32 x, 
                              i32 y, 
                              i32 width, 
                              i32 height);

void app_message_loop(AppState* app);
void app_systems_run(AppState* app);
void app_update_hooks_run(AppState* app); // allows pre-frame-update hooks to run 
void app_update_hooks_install(PreframeUpdateFn fn, i32 priority);
void app_resize(AppState* app, i32 w, i32 h);
void app_shutdown(AppState* app);
void app_begin_frame(AppState* app);
void app_end_frame(AppState* app);
void app_cbuffer_update(AppState* app);

void input_update(InputSystem* input);

/* processes user interaction w/ the scene 
 * calculates selected, hovered, dragging, etc 
 * */
void scene_run_interaction(Scene* scene, InputSystem* input, RayCaster* ray);

/*
 * Render the editor UI for the current scene
 * */
void editor_scene_display(Scene* scene);
void editor_main_ui(AppState* app);
void editor_camera_controls(CameraSystem* camera_system);

/* renders the scene */
void scene_render(Renderer* gfx, FontRenderer* font_sys, Scene* active_scene, f64 frame_delta);

void camera_input(CameraSystem* camera_sys, InputSystem* input, f32 time_delta);

namespace App {

  void start(HINSTANCE hinst, const char *appname);
  void clean_and_exit();
  LRESULT CALLBACK win32_message_callback(HWND hwnd,
                                          UINT msg,
                                          WPARAM wparam,
                                          LPARAM lparam);

  void message_loop();
  void update(float timestep);
  void init_systems();
  void shutdown_systems();

//extern FontRenderer *fonts;
  
//  static Timer timer;
//  static win32::Window main_window;
//  static Renderer renderer;
//  static Scene scene;
//  extern CameraSystem camera_system;

}; // namespace App
 
