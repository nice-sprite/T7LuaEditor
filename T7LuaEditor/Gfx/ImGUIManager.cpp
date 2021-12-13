//
// Created by coxtr on 12/12/2021.
//

#include <fmt/format.h>
#include "ImGUIManager.h"

ImGUIManager::ImGUIManager(ID3D11Device *device, ID3D11DeviceContext *context, HWND hwnd) :
        showSysInfo_(false), showDemoWindow_(true) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    auto font_FantasqueSansMono = io.Fonts->AddFontFromFileTTF(
            "C:/Users/coxtr/source/repos/T7LuaEditor/Resource/Fonts/FantasqueSansMono-Regular.ttf", 14);
    fonts.emplace_back("FantasqueSansMono", font_FantasqueSansMono);
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);
}

ImGUIManager::~ImGUIManager() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGUIManager::RenderUI() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::PushFont(GetFont("FantasqueSansMono")); // this is the main font.

    if (showDemoWindow_)
        ImGui::ShowDemoWindow(&showDemoWindow_);
/*
    if (ImGui::Begin("System Info", &showSysInfo_)) {
        for (auto const &deviceInfoStr: systemInfo.ToString()) {
            ImGui::Text(deviceInfoStr.c_str());
        }
    }
    ImGui::End();

    if (ImGui::Begin("Sequencer Widget")) {

        static int selectedEntry = -1;
        static int firstFrame = 0;
        static bool expanded = true;
        static int currentFrame = 100;

        ImGui::PushItemWidth(130);
        ImGui::InputInt("Frame Min", &timelineSequencer.frameMin_);
        ImGui::SameLine();
        ImGui::InputInt("Frame ", &currentFrame);
        ImGui::SameLine();
        ImGui::InputInt("Frame Max", &timelineSequencer.frameMax_);
        ImGui::PopItemWidth();
        Sequencer(&timelineSequencer, &currentFrame, &expanded, &selectedEntry, &firstFrame,
                  ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL |
                  ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);

    }
    ImGui::End();
    auto timeStr = fmt::format("{}ms\ntotal: {}ms", timeElapsedMs, totalElapsedMs);
    ImGui::GetForegroundDrawList()->AddText(ImVec2(0, 0), 0xFFFFFFFF, timeStr.c_str());
*/
    ImGui::PopFont();
    ImGui::Render();

}

ImFont *ImGUIManager::GetFont(const std::string &fontName) {
    for (auto &f: fonts) {
        if (f.name == fontName)
            return f.font;
    }
    return nullptr;
}
