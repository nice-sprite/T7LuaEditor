#pragma once
#include "font_loader.h"
#include "math.h"
#include "renderer.h"
#include "renderer_types.h"
#include <string>
#include FT_FREETYPE_H
#include <ft2build.h>

#define FONT_DEBUG 1

typedef i32 FontID; 

// represents a font and its location in the atlas
struct FontSubregion {
  Font* font;
  Float4 rect_bounds;
};


struct FontRenderer {
private: 
//  FontLoader loader;
  
  // some made up budget, no idea how much we actually need 
  static constexpr u32 Max_Font_Rects = 2500; 
  static constexpr u32 Max_Vertices = Max_Font_Rects * 4;  
  static constexpr u32 Max_Indices = Max_Font_Rects * 6;  

  static constexpr u32 Vtx_Byte_Size = Max_Vertices * sizeof(VertexPosColorTexcoord);
  static constexpr u32 Index_Byte_Size = Max_Indices * sizeof(u32);

  static constexpr u32 Font_SubAtlas_Max_Width = 2048/2;

  VertexPosColorTexcoord* font_vtx_mem = nullptr;
  u32 next_free_vtx = 0;

  u32* font_idx_mem = nullptr;
  u32 next_free_idx = 0;
 
  /*RENDER RESOURCES*/
  Texture2D atlas; // a big ol' texture to store multiple fonts inside of
  ComPtr<ID3D11VertexShader> font_vertex_shader;
  ComPtr<ID3D11PixelShader> font_pixel_shader;
  ComPtr<ID3D11Buffer> font_vtx_buffer;
  ComPtr<ID3D11Buffer> font_index_buffer;
  ComPtr<ID3D11InputLayout> font_input_layout;

  u32 write_offset_x = 0;
  u32 write_offset_y = 0;
  u32 tallest_in_current_row = 0;

  std::vector<FontSubregion> font_subregions;

  // looks up the glyph info and generates a quad in `quads`
  Float4 generate_quad(const char c, FontSubregion* font_subregion);
  Float4 calculate_uv(const char c, FontSubregion* font_subregion);
  void generate_vertices(Float4 min_max, Float4 uvs, Float4 color);

  void init_render_resources(Renderer* renderer);
  void alloc_font_memory();
  void clear_font_memory();
  void free_font_memory();


  /*FREETYPE LIBRARY*/
  FT_Library freetype_library;
  void freetype_init();
  void freetype_shutdown();
  FT_Face freetype_load_face(fs::path face_path);
  void freetype_free_face(FT_Face face);

public: 

  FontRenderer(Renderer *renderer);

  bool init_atlas_texture(Renderer *renderer);

  FontID load_font(Renderer *renderer,
      std::string path,
      u32 height,
      u32 num_glyphs,
      b8 generate_sdf
      );
  
  void draw_string(char* str, u32 len, Float4 color, FontID font_id = 0);
  
  Texture2D* get_atlas();

  FontSubregion* get_font_by_id(FontID id);

  void submit(Renderer* renderer);

  // quad cache
};
