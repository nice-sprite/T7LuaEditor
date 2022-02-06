#include "../Engine/Renderer.h"
#include "../Engine/SystemInfo.h"
#include "../Engine/Timer.h"
#include "../Engine/WindowUtil.h"
#include "../Engine/Scene.h"


namespace Application
{
    // APP CONFIG
    static constexpr auto AppIcon = L"C:/Users/coxtr/source/repos/T7LuaEditor/T7LuaEditor/appicon.ico";
    static constexpr auto AppWidth = 1920;
    static constexpr auto AppHeight = 1080;
    static constexpr auto AppTitle = "Priscilla";

    void StartApplication(HINSTANCE hinst, const wchar_t* appname);
    LRESULT CALLBACK WndMsgCallback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    void MessageLoop();
    void Tick(float timestep);
    void ImGuiBeginFrame();
    void ImGuiEndFrame();

    static Timer frameTimer;
    static WindowUtil::Window mainWindow;
    static std::unique_ptr<Renderer> rhi = nullptr;
    static std::unique_ptr<Scene2D> scene = nullptr;
    // GameListener gameListener; // listens for POST from game server for live sync
    // EditorUI editor; // editor should have a scene?
};