//
// Created by coxtr on 12/14/2021.
//

#ifndef T7LUAEDITOR_SCENE_H
#define T7LUAEDITOR_SCENE_H

#include "ray_cast.h"
#include "renderer.h"
#include "win32_input.h"
#include <DirectXMath.h>
#include <d3d11.h>
#include <imgui.h>
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
//
//
//  /// ^^^^CHECK if this approach is even reasonable first.
//
// what about layout elements?
// for instance, a horizontal UI list should be usable.
// ui lists are in charge of layout of their children. When we go to render,
// could we have a seperate array of layout widgets like this: struct
// HorizontalListLayout {
//      int index;
//      int children[]; // list of indices for children of this box
// }
//
//
//

struct UIQuad {
  float left, right, top, bottom;
  int texture;
};

struct Selection {
  XMFLOAT4 bounds;
  std::vector<int> quads;
};

class Scene {
private:
  ComPtr<ID3D11Buffer> scene_vertex_buffer;
  ComPtr<ID3D11Buffer> scene_index_buffer;
  ComPtr<ID3D11InputLayout> vertex_layout;
  ComPtr<ID3D11VertexShader> scene_vertex_shader;
  ComPtr<ID3D11PixelShader> scene_pixel_shader;

public:
  static constexpr auto MaxQuads = 10000;
  static constexpr auto MaxIndices =
      6 * MaxQuads; // there are 6 indices per quad

  unsigned __int32 num_quads;
  unsigned __int32 num_dirty;
  XMFLOAT4 bounding_boxs[MaxQuads]{}; // left, right, top, bottom
  XMFLOAT4 rotations[MaxQuads]{};
  XMFLOAT4 colors[MaxQuads]{};
  __int32 priority[MaxQuads]{};
  bool dirty = false;
  /// __int32  dirty_list[MaxQuads]{};

  Scene();

  /// need to make a
  /// vertex buffer
  /// index buffer
  /// vertex and fragment shader, which we store for now
  void init(Renderer &renderer); // create the resources needed to draw

  int add_quad(XMFLOAT4 bounds, XMFLOAT4 color, XMFLOAT4 rotation);

  int get_quad_under_cursor(float x, float y, Camera const &cam);

  void calculate_selected_quads(Camera const &cam);

  void add_lots_of_quads();

  void update(Renderer &renderer, float timestep, Camera &camera);

  void update_resources(Renderer &renderer);

  void draw(Renderer &renderer);

  void tesselate_quads(VertexPosColorTexcoord *mapped_vertex_memory);
  void upload_indices(u32 *mapped_index_buffer);

public:
  Selection selection;

  int width, height;

private:
  void draw_selection(XMFLOAT4 bounds);
};

#endif // T7LUAEDITOR_SCENE_H
