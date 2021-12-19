//
// Created by coxtr on 12/12/2021.
//

#ifndef T7LUAEDITOR_IMGUIMANAGER_H
#define T7LUAEDITOR_IMGUIMANAGER_H

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <Windows.h>
#include <utility>
#include <vector>
#include <string>


// put all state related to the editors UI here
class ImGUIManager {

public:
    struct NamedFont {
        std::string name;
        ImFont* font;
        NamedFont(std::string name, ImFont* font) : name(std::move(name)), font(font) {}
    };
    explicit ImGUIManager(ID3D11Device *device, ID3D11DeviceContext *context, HWND hwnd);
    ~ImGUIManager();

    void RenderUI();
    void BeginUI();
    void EndUI();

private:
    ImFont* GetFont(std::string const& fontName);
    std::vector<NamedFont> fonts;
    bool showSysInfo_ ;
    std::string cpuInfo_;
    std::vector<std::string> gpuInfo_;

    bool showDemoWindow_ ;
};


#endif //T7LUAEDITOR_IMGUIMANAGER_H
