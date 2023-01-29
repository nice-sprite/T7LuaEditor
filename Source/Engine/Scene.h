//
// Created by coxtr on 12/14/2021.
//

#ifndef T7LUAEDITOR_SCENE_H
#define T7LUAEDITOR_SCENE_H

#include "ray_cast.h"
#include "renderer.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <imgui.h>
#include <unordered_map>
#include <vector>

using namespace DirectX;
// class Renderer; // forward declare the renderer

// every UIQuad has
// anchors for top left right bottom
// left, right, top, bottom bounds/position
// x, y, z rotation
// scale&zoom
// rgba
// 5 shaderVectors (vec4) 0, 1, 2, 3, 4 // I do want these because
// writing/porting UI material shaders would be sick parent element, siblings,
// children priority animation time left, animationDuration, animation name UI
// model index
//
// I think text elems are going to be existence based
// if text_elems[current_elem_index] then its a text element
// otherwise its an image element
// ^^^^CHECK if this approach is even reasonable first.
//
// what about layout elements?
// for instance, a horizontal UI list should be usable.
// ui lists are in charge of layout of their children. When we go to render,
// could we have a seperate array of layout widgets like this: struct
// HorizontalListLayout {
//      int index;
//      int children[]; // list of indices for children of this box
// }

struct UIQuad {
  float left, right, top, bottom;
  int texture;
};

struct Selection {
  Float2 min;
  Float2 max;
  std::vector<int> quads;
};


static constexpr auto MaxQuads = 10000;
static constexpr auto MaxWidgetNameSize = 128;
static constexpr auto MaxIndices = 6 * MaxQuads; // there are 6 indices per quad

/* Holds data about the widget hierarchy and layout */
struct SceneDef {
  Float4 bounding_boxs[MaxQuads]{}; // left, right, top, bottom
  Float4 rotations[MaxQuads]{};     // maybe make this a transform Matrix?
  Float4 colors[MaxQuads]{};        // maybe compress this into a u32?
  std::string name[MaxQuads]{};

  u32 element_count; // number of active elements
};

void scene_add_element();

struct GfxSceneResources {
  ID3D11Buffer* vertex_buffer;
  ID3D11Buffer* index_buffer;
  ID3D11InputLayout* vertex_layout;

  // TODO make a "Material" def and live shader reload
  ID3D11VertexShader* vertex_shader;
  ID3D11PixelShader* pixel_shader;
};

/* initialize the renderer resources to render a scene */
void scene_gfx_create(GfxSceneResources* gfx_resources, Renderer* gfx_state, SceneDef* scene);

/* free the resources */
void scene_gfx_destroy(GfxSceneResources* gfx_resources);


class Scene {

public:

  u32 num_quads;
  i32 selected_quad = -1;

  XMFLOAT4 bounding_boxs[MaxQuads]{}; // left, right, top, bottom
  XMFLOAT4 rotations[MaxQuads]{};
  XMFLOAT4 colors[MaxQuads]{};
  std::string name[MaxQuads]{};

  Scene();

  // need to make a
  // vertex buffer
  // index buffer
  // vertex and fragment shader, which we store for now
  void init(Renderer *renderer); // create the resources needed to draw

  int add_quad(XMFLOAT4 bounds, XMFLOAT4 color, XMFLOAT4 rotation);

  int get_quad_under_cursor(ScreenPos);

  void calculate_selected_quads();

  void add_lots_of_quads();

  void update(Renderer *renderer, float timestep);

  void update_resources(Renderer *renderer);

  void draw(Renderer *renderer);

  void tesselate_quads(VertexPosColorTexcoord *mapped_vertex_memory);

  void upload_indices(u32 *mapped_index_buffer);

public:
  Selection selection;
  int width, height;

private:
  void ui_draw_selection();
  void ui_draw_element_list();
};

#endif // T7LUAEDITOR_SCENE_H
