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

// scene keeps track of all the screen quads 
Scene::Scene() : sceneConstants{} {
    add_quad( -720.f, 720.f, -360.f, 360.f, 0);
    Input::Ui::register_callback([this](Input::Ui::MouseState const& mouse, Input::Ui::KeyboardState const& kbd) ->  bool {

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
