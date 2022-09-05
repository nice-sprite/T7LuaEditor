#include "../Engine/renderer.h"
#include "../Engine/scene.h"
#include "../Engine/win32_lib.h"
#include "../Engine/Camera.h"
namespace app 
{
    // APP CONFIG
    static constexpr auto AppIcon = L"w:/T7LuaEditor/T7LuaEditor/appicon.ico";
    static constexpr auto AppWidth = 1920;
    static constexpr auto AppHeight = 1080;
    static constexpr auto AppTitle = "equinox - by nice_sprite";

    void start(HINSTANCE hinst, const wchar_t* appname);
    void clean_and_exit();
    LRESULT CALLBACK win32_message_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    void message_loop();
    void tick(float timestep);
    void draw_scene(float ts);

    static win32::Timer frameTimer;
    static win32::Window mainWindow;
    static Renderer* rhi = nullptr;
    static Scene* scene = nullptr;
   
};
