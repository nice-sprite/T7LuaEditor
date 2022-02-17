//
// Created by coxtr on 12/14/2021.
//

#include "Scene.h"
#include <DirectXMath.h>
#include "Texture.h"
#include "shader_util.h"
using namespace DirectX;
using namespace std::string_literals;

Scene::Scene() : sceneConstants{}
{
    auto mouse_cb = [this](float x, float y, WPARAM flags) -> bool
    {
        static int callcount = 0;
        ImGui::Text("cursor (%f, %f)", x, y);
        ImGui::Text("wparam: %d", flags);
        ImGui::Text("Calls: %d", callcount);
        auto ctrl = input::Ctrl(flags),
             shift = input::Shift(flags),
             btn_left = input::Btn_Left(flags),
             btn_right = input::Btn_Right(flags),
             btn_middle = input::Btn_Mid(flags),
             xbtn1 = input::Btn_XBtn1(flags),
             xbtn2 = input::Btn_XBtn2(flags);

        ImGui::Text("ctrl: %d\n", ctrl);
        ImGui::Text("shift: %d\n", shift);
        ImGui::Text("btn_left: %d\n", btn_left);
        ImGui::Text("btn_right: %d\n", btn_right);
        ImGui::Text("btn_mid: %d\n", btn_middle);
        ImGui::Text("xbtn1: %d\n", xbtn1);
        ImGui::Text("xbtn2: %d\n", xbtn2);
        ++callcount;
        return true;
    };

    auto kbd_cb = [this](input::keyboard_t &keyState) -> bool
    {
        std::string pressedKeys = "";
        for (auto c : keyState.key)
        {
            if (c != 0)
                pressedKeys += c;
        }
        ImGui::Text("%s", pressedKeys.c_str());
        return true;
    };
    input::register_mouse_move_callback(mouse_cb);
    input::register_keyboard_callback(kbd_cb);
}


void Scene::add_quad(float left, 
        float right, 
        float top, 
        float bottom, 
        int texture )
{
    quads[quadCount] = UIQuad{left, right, top, bottom, texture};
    ++quadCount;
}
