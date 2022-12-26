#include "../Engine/camera_system.h"
#include "../Engine/debug_lines.h"
#include "../Engine/font_loader.h"
#include "../Engine/input_system.h"
#include "../Engine/renderer.h"
#include "../Engine/scene.h"
#include "../Engine/timer.h"
#include "../Engine/win32_lib.h"

namespace App {
// APP CONFIG
static constexpr auto DefaultAppWidth = 1920;
static constexpr auto DefaultAppHeight = 1080;
static constexpr auto AppTitle = "equinox - by nice_sprite";

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

extern FontLoader fonts;

static Timer timer;
static win32::Window main_window;
static Renderer renderer;
static Scene scene;
extern CameraSystem camera_system;

extern Renderer::Texture2D test_texture;
}; // namespace App
