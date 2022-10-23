//
// Created by coxtr on 12/14/2021.
//

#include "Scene.h"
#include "Texture.h"
#include "shader_util.h"
#include <random>
#include <windows.h>
#include "ray_cast.h"
#include "win32_input.h"

using namespace DirectX;
using namespace std::string_literals;

// T7 LUIAnimationStateFlags
//
//enum LUIAnimationStateFlags
//{
//    AS_LAYOUT_CACHED = 0x2,
//    AS_STENCIL = 0x4,
//    AS_FOCUS = 0x8,
//    AS_LEFT_PX = 0x40,
//    AS_LEFT_PT = 0x80,
//    AS_ZOOM = 0x100,
//    AS_XROT = 0x200,
//    AS_YROT = 0x400,
//    AS_ZROT = 0x800,
//    AS_RED = 0x1000,
//    AS_GREEN = 0x2000,
//    AS_BLUE = 0x4000,
//    AS_ALPHA = 0x8000,
//    AS_USERDATA_FLOAT = 0x10000,
//    AS_SHADERVECTOR_0 = 0x20000,
//    AS_SHADERVECTOR_1 = 0x40000,
//    AS_SHADERVECTOR_2 = 0x80000,
//    AS_SHADERVECTOR_3 = 0x100000,
//    AS_SHADERVECTOR_4 = 0x200000,
//    AS_SHADERVECTOR_5 = 0x400000,
//    AS_MATERIAL = 0x800000,
//    AS_FONT = 0x1000000,
//    AS_ALIGNMENT = 0x2000000,
//    AS_UI3D_WINDOW = 0x4000000,
//    AS_SCALE = 0x8000000,
//    AS_USE_GAMETIME = 0x10000000,
//    AS_TOP_PT = 0x20000000,
//    AS_TOP_PX = 0x40000000,
//    AS_RIGHT_PT = 0x80000000,
//    AS_RIGHT_PX = 0x100000000,
//    AS_BOTTOM_PT = 0x200000000,
//    AS_BOTTOM_PX = 0x400000000,
//    AS_LETTER_SPACING = 0x1000000000,
//    AS_LINE_SPACING = 0x2000000000,
//    AS_IMAGE = 0x4000000000,
//};
//
//
//struct __attribute__((aligned(8))) LUIElement
//{
//  LUIAnimationState currentAnimationState;
//  LUIElement *parent;
//  LUIElement *prevSibling;
//  LUIElement *nextSibling;
//  LUIElement *firstChild;
//  LUIElement *lastChild;
//  LUIElementLayoutFunction layoutFunction;
//  LUIElementRenderFunction renderFunction;
//  LUIElementMouseFunction mouseFunction;
//  LUIElementCloseFunction closeFunction;
//  LUIAnimationState *prevAnimationState;
//  LUIAnimationState *nextAnimationState;
//  int cacheRef;
//  UIQuadCache *cache;
//  int priority;
//  int textRef;
//  int animationTimeLeft;
//  int animationDuration;
//  int animationNameLuaRef;
//  int strongLuaReference;
//  float left;
//  float top;
//  float right;
//  float bottom;
//  LUIElement::$C4ABE194573AF402BCC947A60729C646 _anon_0;
//  LUIElement::$3F3D4AB6D9209469EE6676535BC976BB _anon_1;
//  LUIElement::$5DD91E8C5F43C9F95B3D72C7D7E9684A _anon_2;
//  float textDimBottom;
//  LUIElement::$D6C9215D993092FBE07E662A6FAC5A1A _anon_3;
//  LUIElement::$A45ED59354BB6FB3558170006C28F313 _anon_4;
//  UIModelIndex model;
//};
//
//struct __attribute__((aligned(8))) LUIAnimationState
//{
//  LUA_MATERIAL_DATATYPE material;
//  TTFDef *font;
//  uint64_t flags;
//  int luaRef;
//  unsigned __int8 tweenFlags;
//  int ui3DWindow;
//  float leftPct;
//  float topPct;
//  float rightPct;
//  float bottomPct;
//  float leftPx;
//  float topPx;
//  float rightPx;
//  float bottomPx;
//  float globalLeft;
//  float globalTop;
//  float globalRight;
//  float globalBottom;
//  float zoom;
//  float xRot;
//  float yRot;
//  float zRot;
//  float scale;
//  float red;
//  float green;
//  float blue;
//  float alpha;
//  vec4_t shaderVector0;
//  vec4_t shaderVector1;
//  vec4_t shaderVector2;
//  vec4_t shaderVector3;
//  vec4_t shaderVector4;
//  LUIAnimationState::$A3CFED264DE7974CE1B7F8FCF2520F75 _anon_0;
//  LUIAnimationState::$2EA9413DFD814DAEF8A90F539116C751 _anon_1;
//  LUIAlignment alignment;
//  unsigned __int32 useGameTime : 1;
//};

// scene keeps track of all the screen quads 
Scene::Scene() {
    num_quads = 0;
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

int Scene::add_quad(XMFLOAT4 bounds, XMFLOAT4 color, XMFLOAT4 rotation) {
    bounding_boxs[num_quads] = bounds;
    colors[num_quads] = color;
    rotations[num_quads] = rotation;
    num_quads++;
    return num_quads-1;
}


void Scene::add_lots_of_quads() {
    const float width = 50.0;
    const float height= 50.0;
    const float padding = 2.f;

    const int n_wide = 5; // how many quads to draw along the X 
    const int n_tall = 5; // and Y

    int x, y, z;

    XMFLOAT4 bb, color, rot;
    float left = 0;
    float right = width;

    float top = 0.f;
    float bottom = height;

    for(int row = 0; row < n_wide; ++row) {
        top += height;
        bottom += height;
        for(int col = 0; col < n_tall; ++col) {

            bb.x = left + padding;
            bb.y = right - padding;
            bb.z = top + padding;
            bb.w = bottom - padding;

            color.x = 1.f - ((float)row/(float)n_wide);
            color.y = (float)col/(float)n_tall;
            color.z = ((float)row/(float)n_wide);
            color.w = 1.0f - (float)col/(float)n_tall;

            rot.x = 0.0;
            rot.y = 0.0;
            rot.z = 0.0;
            rot.w = 0.0;

            add_quad(bb, color, rot);

            left += width ;
            right += width ;
        }
        left = 0.f;
        right = width;
    }
}

void Scene::draw_selection(XMFLOAT4 bounds) {
    auto draw_list = ImGui::GetBackgroundDrawList();
    // imgui does BRG 
    draw_list->AddRectFilled(ImVec2(bounds.x, bounds.z), ImVec2(bounds.y, bounds.w), ImU32(0x10FFBE00));
    draw_list->AddRect(ImVec2(bounds.x, bounds.z), ImVec2(bounds.y, bounds.w), ImU32(0xFFFFBE00));
}

void Scene::update(float timestep, Camera& camera) {
    static int last_quad = -1;
    static XMFLOAT4 bb, col;
    static float time_total = 0.f;
    static bool last_lmb = false;
    int selected_quad; 
    float grow = 1.f;

    if(!last_lmb && Input::GameInput::mouse_button_down(GameInputMouseLeftButton)) {
        selection.bounds.x = Input::Ui::cursor().x;
        selection.bounds.z = Input::Ui::cursor().y;
        last_lmb = true;
    } else if(last_lmb && Input::GameInput::mouse_button_down(GameInputMouseLeftButton)){
        selection.bounds.y = Input::Ui::cursor().x;
        selection.bounds.w = Input::Ui::cursor().y;
        draw_selection(selection.bounds);
    } else {
        last_lmb = false;
    }

    calculate_selected_quads(camera);
    for(auto& q : selection.quads) {
        ImGui::Text("%d", q);
    }

    if(Input::GameInput::mouse_button_down(GameInputMouseLeftButton)) {
        selected_quad = get_quad_under_cursor(
            Input::Ui::cursor().x,
            Input::Ui::cursor().y,
            camera
        );

        if(selected_quad >= 0 && last_quad != selected_quad) {
            if(last_quad >= 0) {
                bounding_boxs[last_quad] = bb;
                colors[last_quad] = col;
            }
        
            // save the stuff
            bb = bounding_boxs[selected_quad];
            col = colors[selected_quad];

            colors[selected_quad].x = 1.0f;
            colors[selected_quad].y = 1.0f;
            colors[selected_quad].z = 1.0f;
            colors[selected_quad].w = 1.0f;

            bounding_boxs[selected_quad].x -= grow;
            bounding_boxs[selected_quad].y += grow;
            bounding_boxs[selected_quad].z -= grow;
            last_quad = selected_quad;
        }
    }
}

void Scene::calculate_selected_quads(Camera const& cam) {
    ray_cast::Ray min_ray; // upper left of the rect
    ray_cast::Ray max_ray; // lower right of the rect
    XMFLOAT4 b = selection.bounds;
    selection.quads.clear();

    min_ray = ray_cast::screen_to_world_ray(b.x, b.z, (float)width, (float)height, cam, XMMatrixIdentity());
    max_ray = ray_cast::screen_to_world_ray(b.y, b.w, (float)width, (float)height, cam, XMMatrixIdentity());

    //add_debug_line(min_ray.origin, XMVectorAdd(min_ray.origin, min_ray.direction), DebugLine::Red);
    
    for(int i = 0; i < num_quads; ++i) {
        if(ray_cast::volume_intersection(min_ray, max_ray, bounding_boxs[i])) {
            selection.quads.push_back(i);
        }
    }
}

int Scene::get_quad_under_cursor(float x, float y, Camera const& cam) {
    ray_cast::Ray r;
    r = ray_cast::screen_to_world_ray(x, y, (float)width, (float)height, cam, XMMatrixIdentity());
    for(int i = 0; i < num_quads; ++i) {
        if(ray_cast::against_quad(r, bounding_boxs[i])) {
            return i;
        }
    }
    return -1;
}
