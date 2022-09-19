#include "../Engine/renderer.h"
#include "../Engine/scene.h"
#include "../Engine/win32_lib.h"
#include "../Engine/Camera.h"
namespace app 
{
    // APP CONFIG
    static constexpr auto DefaultAppWidth = 1920;
    static constexpr auto DefaultAppHeight = 1080;
    static constexpr auto AppTitle = "equinox - by nice_sprite";

    void start(HINSTANCE hinst, const wchar_t* appname);
    void clean_and_exit();
    LRESULT CALLBACK win32_message_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    void message_loop();
    void update(float timestep);

    static win32::Timer timer;
    static win32::Window main_window;
    static Renderer* rhi = nullptr;
    static Scene* scene = nullptr;
};
