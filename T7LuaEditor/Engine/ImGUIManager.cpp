//
// Created by coxtr on 12/12/2021.
//

#include <fmt/format.h>
#include "ImGUIManager.h"
#include "SystemInfo.h"

ImGUIManager::ImGUIManager(ID3D11Device *device, ID3D11DeviceContext *context, HWND hwnd) :
        showSysInfo_(true), showDemoWindow_(true) {
    // Setup Dear ImGui ctx_
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

//    auto font_FantasqueSansMono = io.Fonts->AddFontFromFileTTF(
//            "C:/Users/coxtr/source/repos/T7LuaEditor/Resource/Fonts/FantasqueSansMono-Regular.ttf", 14);
//    fonts.emplace_back("FantasqueSansMono", font_FantasqueSansMono);
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);
    cpuInfo_ = GetCPUInfo();
    gpuInfo_ = ToString(GetGPUInfo());
}

ImGUIManager::~ImGUIManager() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGUIManager::RenderUI() {
    if (showDemoWindow_)
        ImGui::ShowDemoWindow(&showDemoWindow_);

    if (ImGui::Begin("SysInfo", &showSysInfo_)) {

        if (ImGui::CollapsingHeader("CPU")) {
            ImGui::Text("%s", cpuInfo_.c_str());
        }

        if (ImGui::CollapsingHeader("GPU")) {
            for (auto const& infoStr: gpuInfo_)
                ImGui::Text("%s", infoStr.c_str());
        }
    }
    ImGui::End();
}

void ImGUIManager::BeginUI() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    RenderUI();
}

void ImGUIManager::EndUI() {
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

ImFont *ImGUIManager::GetFont(const std::string &fontName) {
    for (auto &f: fonts) {
        if (f.name == fontName)
            return f.font;
    }
    return nullptr;
}
