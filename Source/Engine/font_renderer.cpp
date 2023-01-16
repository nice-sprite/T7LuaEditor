#include "font_renderer.h"
#include "font_loader.h"
#include "files.h"
#include "renderer_types.h"
#include <d3d11.h>
#include <d3dcommon.h>
#include <winnt.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <imgui.h>


FontRenderer::FontRenderer(Renderer *renderer) {
  init_render_resources(renderer);
  alloc_font_memory();
}

bool FontRenderer::init_atlas_texture(Renderer *renderer) {
  TextureParams texdef{};

  texdef.desired_width = ResourceLimits::MaxTextureWidth >> 2;
  texdef.desired_height = ResourceLimits::MaxTextureHeight >> 2;
  texdef.format = TextureFormats::RGBA_8;
  texdef.usage = D3D11_USAGE_DEFAULT;
  texdef.cpu_flags = D3D11_CPU_ACCESS_WRITE;

  bool check = renderer->create_texture(texdef, atlas);
  if (check) {
    LOG_INFO("Created atlas texture for fonts");
  } else {
    LOG_WARNING("Failed to create texture atlas");
  }

  return check;
}

void FontRenderer::init_render_resources(Renderer* renderer) {
  LOG_INFO("FontRenderer creating render resources");
  init_atlas_texture(renderer);
  renderer->create_vertex_buffer(&font_vtx_buffer, Vtx_Byte_Size); 
  renderer->create_index_buffer(&font_index_buffer, Max_Indices); 
  renderer->create_vertex_shader(
      Files::get_shader_root()/"font.hlsl",
      VertexPosColorTexcoord::layout(), 
      &font_vertex_shader, 
      &font_input_layout
  );

  renderer->create_pixel_shader(
      Files::get_shader_root()/"font.hlsl",
      &font_pixel_shader
  );

}

FontID FontRenderer::load_font(Renderer *renderer, std::string path, u32 height) {
  FontSubregion subregion{};
  Font* font =  loader.load_font(path, height, 144, 128);
  subregion.font = font;
  Float4 rect_bounds{};
  u8 *rendered_glyphs = new u8[font->width * font->height * 4];
  u32 freetype_load_mode = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_NORMAL;

  i32 pen_x = 0, pen_y = 0;
  for (int i = 0; i < font->glyph_count; ++i) {
    FT_Error check = FT_Load_Char(font->font_face, i, freetype_load_mode);
    if(check != FT_Err_Ok)
      LOG_WARNING("failed to load the glyph");

    FT_Bitmap *bitmap = &font->font_face->glyph->bitmap;

    if (pen_x + bitmap->width >= font->width) {
      pen_x = 0;
      pen_y += (font->font_face->size->metrics.height >> 6) + 1;
    }

    for (i32 row = 0; row < bitmap->rows; ++row) {
      for (i32 col = 0; col < bitmap->width; ++col) {
        u32 x = (pen_x + col);
        u32 y = (pen_y + row);
        // splat into RGBA format
        rendered_glyphs[ (y * font->width + x) * 4 + 0] = bitmap->buffer[row * bitmap->pitch + col];
        rendered_glyphs[ (y * font->width + x) * 4 + 1] = bitmap->buffer[row * bitmap->pitch + col];
        rendered_glyphs[ (y * font->width + x) * 4 + 2] = bitmap->buffer[row * bitmap->pitch + col];
        rendered_glyphs[ (y * font->width + x) * 4 + 3] = 0xff;
      }
    }
    pen_x += bitmap->width + 1;
  }

#if DEBUG_FONT_LOADER 
  stbi_write_png("debug.png", font->width, font->height, 4, rendered_glyphs, font->width * 4);
#endif


  // we insert row-wise 
  // if the texture is too wide to fit on the end of the current row,
  // we need to increase the insert_at_y offset and set the insert_at_x offset to 0
  // since its a new row
  if(write_offset_x + font->width > atlas.width) {
    write_offset_x = 0;
    write_offset_y += tallest_in_current_row; // go to next row
    tallest_in_current_row = 0; // reset this as we are in a new row
  } 

  if(font->height > tallest_in_current_row)
    tallest_in_current_row = font->height;


  D3D11_BOX region{};
  region.left = write_offset_x;
  region.right = write_offset_x + font->width;
  region.top = write_offset_y;
  region.bottom = write_offset_y + font->height;
  region.front = 0;
  region.back = 1;

  subregion.rect_bounds.x = region.left;
  subregion.rect_bounds.y = region.top;
  subregion.rect_bounds.z = region.right;
  subregion.rect_bounds.w = region.bottom;
  this->font_subregions.push_back(subregion);

  renderer->update_texture_subregion(atlas,  
                                    0,
                                    &region,
                                    rendered_glyphs,
                                    font->width * 4);

                                    
 // renderer.update_texture(atlas, [=](u8 *buffer, u32 row_pitch) {
 //   u8 *rg = rendered_glyphs;
 //   for (int row = 0; row < font->height; ++row) {
 //     memcpy(buffer  + ( ( write_offset_y * 4 ) + ( write_offset_x * 4 )), rg, font->width * 4);
 //     buffer += row_pitch;
 //     rg += font->width * 4;
 //   }
 // });

  write_offset_x += font->width;

  delete[] rendered_glyphs; // free the temp data
  return this->font_subregions.size()-1;
}

void FontRenderer::draw_string(char* str, u32 len, Float4 color, FontID font_id) {
  FontSubregion* font;
  font = get_font_by_id(font_id);

  if(!str) return;
  if(!font) return;

  ImDrawList* window_draw = ImGui::GetWindowDrawList();
  ImVec2 window_pos = ImGui::GetWindowPos();
  window_pos.x += 6;
  window_pos.y += 40;

  for(int i = 0; i < len; ++i) {
    Float4 quad = this->generate_quad(str[i], font);
    Float4 uvs = this->calculate_uv(str[i], font);
    this->generate_vertices(quad, uvs, color);
    quad.x += window_pos.x;
    quad.y += window_pos.y;
    quad.z += window_pos.x;
    quad.w += window_pos.y;
    window_draw->AddRect(ImVec2(quad.x, quad.y), ImVec2(quad.z, quad.w), IM_COL32(255, 0, 0, 255));
  }
}

// generates a non-transformed quad that is simply the of the glyph
// will need to be transformed/layout'd 
Float4 FontRenderer::generate_quad(const char c, FontSubregion* font) {
  Float4 quad{};
  GlyphInfo glyph_info = font->font->glyph_info[c];
  quad.x = glyph_info.x0;
  quad.y = glyph_info.y0;
  quad.z = glyph_info.x1;
  quad.w = glyph_info.y1;
  return quad;
}

Texture2D* FontRenderer::get_atlas() {
  return &atlas;
}

FontSubregion* FontRenderer::get_font_by_id(FontID id) {
  if(id >=  font_subregions.size()) return nullptr;
  return &font_subregions[id];
}

Float4 FontRenderer::calculate_uv(const char c, FontSubregion* font_subregion) {
  GlyphInfo const * const glyph = &font_subregion->font->glyph_info[c];
  Float4 uv;


  // calculate min
  uv.x = glyph->x0 + font_subregion->rect_bounds.x;
  uv.y = glyph->y0 + font_subregion->rect_bounds.y;

  // calculate max
  uv.z = glyph->x1 + font_subregion->rect_bounds.x;
  uv.w = glyph->y1 + font_subregion->rect_bounds.y;

  uv.x /= this->atlas.width;
  uv.y /= this->atlas.height;
  uv.z /= this->atlas.width;
  uv.w /= this->atlas.height;
  return uv;
}

// TODO do not use vector for storing vert/index
void FontRenderer::generate_vertices(Float4 min_max, Float4 uvs, Float4 color) {
  if(this->next_free_vtx + 4 > Max_Vertices) {
    LOG_WARNING("exceeded vertex budget for font quads!!! {}", this->next_free_vtx);
    return;
  }

  if(this->next_free_idx + 6 > Max_Indices) {
    LOG_WARNING("exceeded index budget for font quads!!! {}", this->next_free_idx);
    return;
  }

  this->font_vtx_mem[this->next_free_vtx + 0] = VertexPosColorTexcoord {
    Float3{min_max.x, min_max.y, 0.0f},
    color, 
    Float2{uvs.x, uvs.w}
  };

  this->font_vtx_mem[this->next_free_vtx + 1] = VertexPosColorTexcoord {
    Float3{min_max.z, min_max.y, 0.0f},
    color, 
    Float2{uvs.z, uvs.w}
  };


  this->font_vtx_mem[this->next_free_vtx + 2] = VertexPosColorTexcoord {
    Float3{min_max.x, min_max.w, 0.0f},
    color, 
    Float2{uvs.x, uvs.y}
  };

  this->font_vtx_mem[this->next_free_vtx + 3] = VertexPosColorTexcoord {
    Float3{min_max.z, min_max.w, 0.0f},
    color, 
    Float2{uvs.z, uvs.y}
  };

  this->next_free_vtx += 4;

  u32 num_quads = this->next_free_vtx/4;
  this->font_idx_mem[this->next_free_idx + 0] = num_quads * 4 + 2;
  this->font_idx_mem[this->next_free_idx + 1] = num_quads * 4 + 3;
  this->font_idx_mem[this->next_free_idx + 2] = num_quads * 4 + 1;
  this->font_idx_mem[this->next_free_idx + 3] = num_quads * 4 + 2;
  this->font_idx_mem[this->next_free_idx + 4] = num_quads * 4 + 1;
  this->font_idx_mem[this->next_free_idx + 5] = num_quads * 4 + 0;
  this->next_free_idx += 6;
}

void FontRenderer::submit(Renderer* renderer) {
  renderer->update_buffer(font_vtx_buffer.Get(), [this](D3D11_MAPPED_SUBRESOURCE& msr) {
        memcpy(msr.pData,
               this->font_vtx_mem, 
               this->next_free_vtx * sizeof(VertexPosColorTexcoord)
        );
  });

  renderer->update_buffer(font_index_buffer.Get(), [this](D3D11_MAPPED_SUBRESOURCE& msr) {
        memcpy(msr.pData, 
               this->font_idx_mem,
               this->next_free_idx * sizeof(u32)
        );
  });

  renderer->set_texture(&atlas);
  renderer->set_topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  renderer->set_input_layout(this->font_input_layout.Get());
  renderer->set_pixel_shader(this->font_pixel_shader.Get());
  renderer->set_vertex_shader(this->font_vertex_shader.Get());
  renderer->set_vertex_buffer(
      this->font_vtx_buffer.GetAddressOf(),
      1,
      sizeof(VertexPosColorTexcoord),
      0
  );

  renderer->set_index_buffer(
      this->font_index_buffer.Get()
  );

  renderer->draw_indexed(this->next_free_idx);
  // flush the vectors
  clear_font_memory();
}


void FontRenderer::alloc_font_memory() {
  this->font_vtx_mem = (VertexPosColorTexcoord*)VirtualAlloc(nullptr,
      Vtx_Byte_Size, 
      MEM_COMMIT | MEM_RESERVE,
      PAGE_READWRITE
  );

  this->font_idx_mem = (u32*)VirtualAlloc(nullptr,
      Index_Byte_Size, 
      MEM_COMMIT | MEM_RESERVE,
      PAGE_READWRITE
  );

  Q_ASSERT(this->font_vtx_mem != nullptr);
  Q_ASSERT(this->font_idx_mem != nullptr);
}

void FontRenderer::free_font_memory() {
  if(this->font_vtx_mem) {
    VirtualFree(this->font_vtx_mem, Vtx_Byte_Size, MEM_DECOMMIT | MEM_RELEASE);
    this->font_vtx_mem = nullptr;
  }

  if(this->font_idx_mem) {
    VirtualFree(this->font_idx_mem, Index_Byte_Size, MEM_DECOMMIT | MEM_RELEASE);
    this->font_idx_mem = nullptr;
  }
}

// resets the buffers to 0
void FontRenderer::clear_font_memory() {

  if(!this->font_vtx_mem) return;
  if(!this->font_idx_mem) return;

  memset(this->font_vtx_mem, 0, Vtx_Byte_Size);
  memset(this->font_idx_mem, 0, Index_Byte_Size);
  this->next_free_idx = 0;
  this->next_free_vtx = 0;

}
