//
// Created by coxtr on 12/14/2021.
//

#ifndef T7LUAEDITOR_SCENE_H
#define T7LUAEDITOR_SCENE_H
#include "gpu_resources.h"
#include "win32_input.h"
#include <imgui.h>
#include <vector>

struct UIQuad
{
    float left, right, top, bottom;
    int texture;
};

class Scene {

public:
    static constexpr auto MaxQuads = 10000;
    static constexpr auto MaxIndices = 6 * MaxQuads; // there are 6 indices per quad

    Scene();

    void add_quad(float left, 
        float right, 
        float top, 
        float bottom, 
        int texture = 0
    );

    void draw_scene(float timestep);

public:
    size_t quadCount = 0;
    std::array<UIQuad, MaxQuads> quads; 
    
private:
    int width, height;
};

#endif //T7LUAEDITOR_SCENE_H
