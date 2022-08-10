//
// Created by coxtr on 12/14/2021.
//

#include "Scene.h"
#include <DirectXMath.h>
#include "Texture.h"
#include "shader_util.h"
#include <random>
using namespace DirectX;
using namespace std::string_literals;

Scene::Scene() : sceneConstants{} {
    add_quad( -720.f, 720.f, -360.f, 360.f,0);
    input::register_keyboard_callback([this](input::keyboard_t &keys) -> bool {
        static int last_state = 0;
        std::random_device rd{};
        std::mt19937 engine{rd()};
        std::uniform_real_distribution<float> dist{-200.0f, 200.0f};
        float rand_x = dist(engine);
        float rand_y = dist(engine);

        if(last_state == 0 && keys.key['B']) {
            last_state = 1;
            this->add_quad(-750 + rand_x, -600.0 + rand_x, -150.f + rand_y, 0.0f + rand_y, 0);
        } 
        if (last_state == 1 && !keys.key['B']) {
            last_state = 0;
        }
        return true;
    });

    input::register_mouse_move_callback([this](float x, float y, WPARAM extra) -> bool {
        ImGui::Text("mouse: (%f %f)", x, y);
        return true;
    });
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


void Scene::draw_scene(float timestep) {
    for(auto quad : quads) {

    }
}