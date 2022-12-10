//
// Created by coxtr on 12/14/2021.
//

#include "scene.h"
#include "debug_lines.h"
#include "imgui_fmt.h"
#include "input_system.h"
#include "ray_cast.h"
#include "shader_util.h"
#include "texture.h"
#include <DirectXMath.h>
#include <d3dcommon.h>
#include <random>
#include <unordered_map>
#include <windows.h>

using namespace DirectX;
using namespace std::string_literals;

/*
T7 LUIAnimationStateFlags

enum LUIAnimationStateFlags
{
   AS_LAYOUT_CACHED = 0x2,
   AS_STENCIL = 0x4,
   AS_FOCUS = 0x8,
   AS_LEFT_PX = 0x40,
   AS_LEFT_PT = 0x80,
   AS_ZOOM = 0x100,
   AS_XROT = 0x200,
   AS_YROT = 0x400,
   AS_ZROT = 0x800,
   AS_RED = 0x1000,
   AS_GREEN = 0x2000,
   AS_BLUE = 0x4000,
   AS_ALPHA = 0x8000,
   AS_USERDATA_FLOAT = 0x10000,
   AS_SHADERVECTOR_0 = 0x20000,
   AS_SHADERVECTOR_1 = 0x40000,
   AS_SHADERVECTOR_2 = 0x80000,
   AS_SHADERVECTOR_3 = 0x100000,
   AS_SHADERVECTOR_4 = 0x200000,
   AS_SHADERVECTOR_5 = 0x400000,
   AS_MATERIAL = 0x800000,
   AS_FONT = 0x1000000,
   AS_ALIGNMENT = 0x2000000,
   AS_UI3D_WINDOW = 0x4000000,
   AS_SCALE = 0x8000000,
   AS_USE_GAMETIME = 0x10000000,
   AS_TOP_PT = 0x20000000,
   AS_TOP_PX = 0x40000000,
   AS_RIGHT_PT = 0x80000000,
   AS_RIGHT_PX = 0x100000000,
   AS_BOTTOM_PT = 0x200000000,
   AS_BOTTOM_PX = 0x400000000,
   AS_LETTER_SPACING = 0x1000000000,
   AS_LINE_SPACING = 0x2000000000,
   AS_IMAGE = 0x4000000000,
};


struct __attribute__((aligned(8))) LUIElement
{
 LUIAnimationState currentAnimationState;
 LUIElement *parent;
 LUIElement *prevSibling;
 LUIElement *nextSibling;
 LUIElement *firstChild;
 LUIElement *lastChild;
 LUIElementLayoutFunction layoutFunction;
 LUIElementRenderFunction renderFunction;
 LUIElementMouseFunction mouseFunction;
 LUIElementCloseFunction closeFunction;
 LUIAnimationState *prevAnimationState;
 LUIAnimationState *nextAnimationState;
 int cacheRef;
 UIQuadCache *cache;
 int priority;
 int textRef;
 int animationTimeLeft;
 int animationDuration;
 int animationNameLuaRef;
 int strongLuaReference;
 float left;
 float top;
 float right;
 float bottom;
 LUIElement::$C4ABE194573AF402BCC947A60729C646 _anon_0;
 LUIElement::$3F3D4AB6D9209469EE6676535BC976BB _anon_1;
 LUIElement::$5DD91E8C5F43C9F95B3D72C7D7E9684A _anon_2;
 float textDimBottom;
 LUIElement::$D6C9215D993092FBE07E662A6FAC5A1A _anon_3;
 LUIElement::$A45ED59354BB6FB3558170006C28F313 _anon_4;
 UIModelIndex model;
};

struct __attribute__((aligned(8))) LUIAnimationState
{
 LUA_MATERIAL_DATATYPE material;
 TTFDef *font;
 uint64_t flags;
 int luaRef;
 unsigned __int8 tweenFlags;
 int ui3DWindow;
 float leftPct;
 float topPct;
 float rightPct;
 float bottomPct;
 float leftPx;
 float topPx;
 float rightPx;
 float bottomPx;
 float globalLeft;
 float globalTop;
 float globalRight;
 float globalBottom;
 float zoom;
 float xRot;
 float yRot;
 float zRot;
 float scale;
 float red;
 float green;
 float blue;
 float alpha;
 vec4_t shaderVector0;
 vec4_t shaderVector1;
 vec4_t shaderVector2;
 vec4_t shaderVector3;
 vec4_t shaderVector4;
 LUIAnimationState::$A3CFED264DE7974CE1B7F8FCF2520F75 _anon_0;
 LUIAnimationState::$2EA9413DFD814DAEF8A90F539116C751 _anon_1;
 LUIAlignment alignment;
 unsigned __int32 useGameTime : 1;
};
*/

// scene keeps track of all the screen quads
Scene::Scene() { num_quads = 0; }

void Scene::init(Renderer &renderer) {
  fs::path quad_shader = Files::get_shader_root() / "TexturedQuad.hlsl";
  renderer.create_vertex_buffer(&scene_vertex_buffer,
                                4 * sizeof(VertexPosColorTexcoord) * MaxQuads);
  renderer.create_index_buffer(&scene_index_buffer, 6 * MaxQuads);
  renderer.create_vertex_shader(quad_shader,
                                VertexPosColorTexcoord::layout(),
                                &scene_vertex_shader,
                                &vertex_layout);
  renderer.create_pixel_shader(quad_shader, &scene_pixel_shader);

  MouseEventListener test_listener{};
  test_listener.self = (void *)this;
  test_listener.function = (void *)[](void *self, MouseEvent e) {
    auto *me = (Scene *)self;
    // Dragging test uwu
    switch (e.type) {
    case DragStart:
      LOG_INFO("DragStart @ ({} {})", e.mouse_pos.x, e.mouse_pos.y);
      me->selection.min = e.mouse_pos;
      me->selection.max = e.mouse_pos;
      break;
    case Dragging:
      LOG_INFO("Dragging @ ({} {})", e.mouse_pos.x, e.mouse_pos.y);
      me->selection.max = e.mouse_pos;
      break;
    case DragEnd:
      LOG_INFO("DragEnd @ ({} {})", e.mouse_pos.x, e.mouse_pos.y);
      me->selection.min.x = 0;
      me->selection.min.y = 0;

      me->selection.max.x = 0;
      me->selection.max.y = 0;
      break;
    case MouseLeftDblClick:
      LOG_INFO("MouseLeftDblClick", e.mouse_pos.x, e.mouse_pos.y);
      me->selected_quad = me->get_quad_under_cursor(e.mouse_pos);

    default:
      break;
    }
  };

  if (InputSystem::instance().add_mouse_listener(test_listener)) {
    LOG_INFO("(scene) added mouse handler");
  }
}

int Scene::add_quad(XMFLOAT4 bounds, XMFLOAT4 color, XMFLOAT4 rotation) {
  root_data.bounding_boxs[num_quads] = bounds;
  root_data.colors[num_quads] = color;
  root_data.rotations[num_quads] = rotation;
  num_quads++;
  return num_quads - 1;
}

void Scene::add_lots_of_quads() {
  const float width = 50.0;
  const float height = 50.0;
  const float padding = 2.f;

  const int n_wide = 1; // how many quads to draw along the X
  const int n_tall = 1; // and Y

  int x, y, z;

  XMFLOAT4 bb, color, rot;
  float left = 0;
  float right = width;

  float top = 0.f;
  float bottom = height;

  for (int row = 0; row < n_wide; ++row) {
    top += height;
    bottom += height;
    for (int col = 0; col < n_tall; ++col) {

      bb.x = left + padding;
      bb.y = right - padding;
      bb.z = top + padding;
      bb.w = bottom - padding;

      color.x = 1.f - ((float)row / (float)n_wide);
      color.y = (float)col / (float)n_tall;
      color.z = ((float)row / (float)n_wide);
      color.w = 1.0f - (float)col / (float)n_tall;

      rot.x = 0.0;
      rot.y = 0.0;
      rot.z = 0.0;
      rot.w = 0.0;

      add_quad(bb, color, rot);

      left += width;
      right += width;
    }
    left = 0.f;
    right = width;
  }
}

void Scene::ui_draw_selection() {
  if (InputSystem::instance().imgui_active)
    return;
  auto draw_list = ImGui::GetBackgroundDrawList();
  // imgui does BRG

  draw_list->AddRectFilled(ImVec2(selection.min.x, selection.min.y),
                           ImVec2(selection.max.x, selection.max.y),
                           ImU32(0x10FFBE00));

  draw_list->AddRect(ImVec2(selection.min.x, selection.min.y),
                     ImVec2(selection.max.x, selection.max.y),
                     ImU32(0xFFFFBE00));

  // draw the shid
  // draw_list->AddText(ImVec2(bounds.x - 12.0f, bounds.z - 12.0f),
  //                    ImU32(0xFFFFBE00),
  //                    fmt::format("{}, {}", bounds.x, bounds.z).c_str());

  // draw_list->AddText(ImVec2(bounds.y + 12.0f, bounds.w + 12.0f),
  //                    ImU32(0xFFFFBE00),
  //                    fmt::format("{}, {}", bounds.y, bounds.w).c_str());
}

void Scene::update(Renderer &renderer, float timestep) {
  InputSystem &input = InputSystem::instance();
  static i32 last_quad = -1;
  static XMFLOAT4 bb, col;
  static f32 time_total = 0.f;
  f32 grow = 1.f;

  ui_draw_selection();
  // calculate_selected_quads();

  // selected_quad = get_quad_under_cursor(InputSystem::instance().mouse_pos);
  if (this->selected_quad >= 0) {
    root_data.colors[selected_quad] = colors[DebugColors::Blue];
  }

  update_resources(renderer);
}

void Scene::calculate_selected_quads() {
  Ray min_ray; // upper left of the rect
  Ray max_ray; // lower right of the rect

  selection.quads.clear();

  min_ray = RayCaster::instance().picking_ray(XMLoadFloat2(&selection.min));
  min_ray = RayCaster::instance().picking_ray(XMLoadFloat2(&selection.max));
  // min_ray = ray_cast::screen_to_world_ray(b.x,
  //                                         b.z,
  //                                         (float)width,
  //                                         (float)height,
  //                                         cam,
  //                                         XMMatrixIdentity());

  // max_ray = ray_cast::screen_to_world_ray(b.y,
  //                                         b.w,
  //                                         (float)width,
  //                                         (float)height,
  //                                         cam,
  //                                         XMMatrixIdentity());

  // ImGui::TextFmt("2d selection: {}", b);
  ImGui::TextFmt("min: {}\n", min_ray);
  ImGui::TextFmt("max: {}\n", max_ray);
#if 0
  if (Input::GameInput::key_down(VK_SPACE)) {
    // DebugRenderSystem::instance().clear_debug_lines();
    DebugRenderSystem::instance().debug_ray(min_ray);
    DebugRenderSystem::instance().debug_ray(max_ray);
  }
#endif

  //  for (int i = 0; i < num_quads; ++i) {
  //    if (ray_cast::volume_intersection(min_ray,
  //                                      max_ray,
  //                                      root_data.bounding_boxs[i])) {
  //      selection.quads.push_back(i);
  //    }
  //  }
}

int Scene::get_quad_under_cursor(ScreenPos mouse) {
  Ray r;
  r = RayCaster::instance().picking_ray(XMLoadFloat2(&mouse));
  DebugRenderSystem::instance().debug_ray(r);

  for (int i = 0; i < num_quads; ++i) {
    if (RayCaster::instance().ray_quad(r, root_data.bounding_boxs[i])) {
      return i;
    }
  }
  return -1;
}

void Scene::draw(Renderer &renderer) {
  renderer.set_vertex_buffer(scene_vertex_buffer.GetAddressOf(),
                             1,
                             sizeof(VertexPosColorTexcoord),
                             0);
  renderer.set_index_buffer(scene_index_buffer.Get());
  renderer.set_pixel_shader(scene_pixel_shader.Get());
  renderer.set_vertex_shader(scene_vertex_shader.Get());
  renderer.set_input_layout(vertex_layout.Get());
  renderer.set_topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  renderer.draw_indexed(num_quads * 6);
}

void Scene::update_resources(Renderer &renderer) {
  renderer.update_buffer(scene_vertex_buffer.Get(),
                         [=](D3D11_MAPPED_SUBRESOURCE &mapped_mem) {
                           tesselate_quads(
                               (VertexPosColorTexcoord *)mapped_mem.pData);
                         });

  renderer.update_buffer(scene_index_buffer.Get(),
                         [=](D3D11_MAPPED_SUBRESOURCE &mapped_mem) {
                           upload_indices((u32 *)mapped_mem.pData);
                         });
}

void Scene::tesselate_quads(VertexPosColorTexcoord *mapped_vertex_memory) {
  for (i32 i = 0; i < num_quads; ++i) {
    XMFLOAT4 pos = root_data.bounding_boxs[i];
    XMFLOAT4 color = root_data.colors[i];
    mapped_vertex_memory[i * 4 + 0] = {DirectX::XMFLOAT3{pos.x, pos.z, 0.f},
                                       color,
                                       DirectX::XMFLOAT2{0.0f, 0.0f}};
    mapped_vertex_memory[i * 4 + 1] = {DirectX::XMFLOAT3{pos.y, pos.z, 0.f},
                                       color,
                                       DirectX::XMFLOAT2{0.0f, 0.0f}};
    mapped_vertex_memory[i * 4 + 2] = {DirectX::XMFLOAT3{pos.x, pos.w, 0.f},
                                       color,
                                       DirectX::XMFLOAT2{0.0f, 0.0f}};
    mapped_vertex_memory[i * 4 + 3] = {DirectX::XMFLOAT3{pos.y, pos.w, 0.f},
                                       color,
                                       DirectX::XMFLOAT2{0.0f, 0.0f}};
  }
}

void Scene::upload_indices(u32 *mapped_index_buffer) {
  for (i32 i = 0; i < num_quads; ++i) {
    mapped_index_buffer[i * 6 + 0] = i * 4 + 2;
    mapped_index_buffer[i * 6 + 1] = i * 4 + 3;
    mapped_index_buffer[i * 6 + 2] = i * 4 + 1;
    mapped_index_buffer[i * 6 + 3] = i * 4 + 2;
    mapped_index_buffer[i * 6 + 4] = i * 4 + 1;
    mapped_index_buffer[i * 6 + 5] = i * 4 + 0;
  }
}
