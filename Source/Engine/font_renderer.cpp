#include "font_renderer.h"
#include "files.h"
#include "renderer_types.h"
#include <d3d11.h>
#include <d3dcommon.h>
#include <winnt.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <imgui.h>


FontRenderer::FontRenderer(Renderer *renderer) {
  freetype_init();
  init_render_resources(renderer);
  alloc_font_memory();

  this->sdf_spread = 4; 
  this->sdf_overlaps = true;
}

void FontRenderer::freetype_init() {
  FT_Error e = FT_Init_FreeType(&this->freetype_library);
  LOG_INFO("init freetype... {}", FT_Error_String(e));
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

FontID FontRenderer::load_font(Renderer *renderer, std::string path, u32 height, u32 num_glyphs, b8 generate_sdf) {

  FontSubregion subregion{};

  /* check that the filepath exists and is accesibile */
  fs::path ttf_path = path;
  if(!Files::file_exists(ttf_path)) {
    LOG_WARNING("{} either does not exist or is not accesibile under current permissions level", ttf_path.string());
    return -1;
  }

  /* load up the FT_Face for the font */
  FT_Face face;
  FT_Error check;
  check = FT_New_Face(this->freetype_library, ttf_path.string().c_str(), 0, &face);

  if(check != FT_Err_Ok) {
    LOG_WARNING("error loading {}: {}", ttf_path.string(), FT_Error_String(check));
    return -1;
  }

  /* setup the font subregion */
  subregion.font = new TTF(face);
  subregion.font->line_height = 0;
  subregion.font->glyph_count = num_glyphs;
  subregion.font->glyph_info = (GlyphInfo*)VirtualAlloc(nullptr, num_glyphs * sizeof(GlyphInfo), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

  /* Set the size to the requested size */
  check = FT_Set_Pixel_Sizes(face, 0, (FT_UInt)height);
  if(check !=  FT_Err_Ok) {
    LOG_WARNING("FT_Set_Pixel_Sizes on {}: {}", ttf_path.string(), FT_Error_String(check));
    return -1;
  }

  FT_Property_Set( this->freetype_library, "bsdf", "spread", &this->sdf_spread);
  FT_Property_Set( this->freetype_library, "sdf", "spread", &this->sdf_spread );
  FT_Property_Set( this->freetype_library, "sdf", "overlaps", &this->sdf_overlaps);

  /* Calculate a tight bounding box for the atlas */
  //TODO filter out non-displayable characters for a smaller bb

  i32 bbox_width;
  i32 bbox_height;

  i32 pen_x = 0, pen_y = 0;
  i32 tallest_glyph_in_row = 0;
  i32 widest_glyph = 0;
  i32 widest_row = 0;
  FT_Int32 load_params = FT_LOAD_DEFAULT; 
  FT_Int32 render_params = generate_sdf ? FT_RENDER_MODE_SDF : FT_RENDER_MODE_NORMAL;
  //render_params = FT_RENDER_MODE_NORMAL;

  for(i32 i = Start_Glyph; i < num_glyphs; ++i) {
    if(i == ' ') continue;
    FT_UInt glyph_index = FT_Get_Char_Index(face, i);
    if(glyph_index == 0) {
      LOG_WARNING("encountered undefined character code for char \'{}\'", i);
      continue;
    }
    check = FT_Load_Glyph(face, glyph_index, load_params);
    if(check != FT_Err_Ok) {
      LOG_WARNING("FT_Load_Glyph on {}: {}", ttf_path.string(), FT_Error_String(check));
      return -1; // can maybe continue?
    }

    if(load_params | FT_RENDER_MODE_SDF) {
      // LOG_INFO("rendering SDF glyphs for {}", ttf_path.string());
    } else {
      // LOG_INFO("rendering normal glyphs for {}", ttf_path.string());
    }
    
    if(Use_Bitmap_SDF) {
      check = FT_Render_Glyph(face->glyph, (FT_Render_Mode)FT_RENDER_MODE_NORMAL);
      if(check != FT_Err_Ok) {
        LOG_WARNING("FT_Render_Glyph on {}: {}", ttf_path.string(), FT_Error_String(check));
        return -1;
      }
    }
    check = FT_Render_Glyph(face->glyph, (FT_Render_Mode)render_params);
    if(check != FT_Err_Ok) {
      LOG_WARNING("FT_Render_Glyph on {}: {}", ttf_path.string(), FT_Error_String(check));
      return -1;
    }

    FT_Bitmap* bitmap = &face->glyph->bitmap;

    if(face->glyph->bitmap.rows > tallest_glyph_in_row) {
      tallest_glyph_in_row = face->glyph->bitmap.rows;
    }

    if(face->glyph->bitmap.rows > subregion.font->line_height) {
      subregion.font->line_height = face->glyph->bitmap.rows;
    }

    if(face->glyph->bitmap.width > widest_glyph) {
      widest_glyph = face->glyph->bitmap.width;
    }

    // if we have exceeded the length of a row in the atlas 
    if(pen_x + face->glyph->bitmap.width > Font_SubAtlas_Max_Width) {

      pen_y += tallest_glyph_in_row+1;
      pen_x = 0;
    }

    if(pen_x > widest_row)
      widest_row = pen_x;

    /* extract glyph bounding box for rendering later */
    subregion.font->glyph_info[i].x0 = pen_x;
    subregion.font->glyph_info[i].y0 = pen_y;
    subregion.font->glyph_info[i].x1 = pen_x + bitmap->width;
    subregion.font->glyph_info[i].y1 = pen_y + bitmap->rows;
    subregion.font->glyph_info[i].x_offset = face->glyph->bitmap_left;
    subregion.font->glyph_info[i].y_offset = face->glyph->bitmap_top;
    subregion.font->glyph_info[i].advance.x = face->glyph->advance.x >> 6;
    subregion.font->glyph_info[i].advance.y = face->glyph->advance.y >> 6;

    pen_x += face->glyph->bitmap.width;
  }

  bbox_width = mini32(Font_SubAtlas_Max_Width, widest_row);
  bbox_height = pen_y + tallest_glyph_in_row;
  LOG_INFO("calculated atlas_bbox: {} {}", 
      bbox_width,
      bbox_height
      );


  /* Allocate memory for the bitmap and render to it */
  u8* rendered_glyph_buffer = (u8*)VirtualAlloc(nullptr, // TODO add my own tracking allocator
      bbox_width * bbox_height * 4, 
      MEM_COMMIT | MEM_RESERVE, 
      PAGE_READWRITE
      );

  pen_x = 0; 
  pen_y = 0;
  tallest_glyph_in_row = 0;
  widest_glyph = 0;

  for(i32 i = Start_Glyph; i < num_glyphs; ++i) {
    if(i == ' ') continue;
    FT_UInt glyph_index = FT_Get_Char_Index(face, i);
    if(glyph_index == 0) {
      LOG_WARNING("encountered undefined character code for char \'{}\'", i);
      continue;
    }
    check = FT_Load_Glyph(face, glyph_index, load_params);
    if(check != FT_Err_Ok) {
      LOG_WARNING("FT_Load_Glyph on {}: {}", ttf_path.string(), FT_Error_String(check));
      return -1; // can maybe continue?
    }
    
    if(Use_Bitmap_SDF) {
      check = FT_Render_Glyph(face->glyph, (FT_Render_Mode)FT_RENDER_MODE_NORMAL);
      if(check != FT_Err_Ok) {
        LOG_WARNING("FT_Render_Glyph on {}: {}", ttf_path.string(), FT_Error_String(check));
        return -1;
      }
    }
    check = FT_Render_Glyph(face->glyph, (FT_Render_Mode)render_params);
    if(check != FT_Err_Ok) {
      LOG_WARNING("FT_Render_Glyph on {}: {}", ttf_path.string(), FT_Error_String(check));
      return -1;
    }

    FT_Bitmap* bitmap = &face->glyph->bitmap;

    if(face->glyph->bitmap.rows > tallest_glyph_in_row) {
      tallest_glyph_in_row = face->glyph->bitmap.rows;
    }

    if(face->glyph->bitmap.width > widest_glyph) {
      widest_glyph = face->glyph->bitmap.width;
    }

    // if we have exceeded the length of a row in the atlas 
    if(pen_x + face->glyph->bitmap.width > Font_SubAtlas_Max_Width) {
      pen_x = 0;
      // SDFs may have a padding that comes from the "spread" that this does 
      // not actually account for, so maybe use max(bitmap->height, metrics.height)
      pen_y += tallest_glyph_in_row+1;
    }

    for (i32 row = 0; row < bitmap->rows; ++row) {
      for (i32 col = 0; col < bitmap->width; ++col) {
        u32 x = (pen_x + col);
        u32 y = (pen_y + row);
        // splat into RGBA format
        rendered_glyph_buffer [ (y * bbox_width + x) * 4 + 0] = bitmap->buffer[row * bitmap->pitch + col];
        rendered_glyph_buffer [ (y * bbox_width + x) * 4 + 1] = bitmap->buffer[row * bitmap->pitch + col];
        rendered_glyph_buffer [ (y * bbox_width + x) * 4 + 2] = bitmap->buffer[row * bitmap->pitch + col];
        rendered_glyph_buffer [ (y * bbox_width + x) * 4 + 3] = bitmap->buffer[row * bitmap->pitch + col];
      }
    }
    pen_x += face->glyph->bitmap.width;
  }


#if FONT_DEBUG
  std::string family_name = std::string(face->family_name);
  auto file_name = fmt::format("DEBUG_{}_{}x{}.png", family_name, bbox_width, bbox_height);
  stbi_write_png(file_name.c_str(), bbox_width, bbox_height, 4, rendered_glyph_buffer, bbox_width * 4);
#endif

  // return 1;

  // we insert row-wise 
  // if the texture is too wide to fit on the end of the current row,
  // we need to increase the insert_at_y offset and set the insert_at_x offset to 0
  // since its a new row
  if(write_offset_x + bbox_width > atlas.width) {
    write_offset_x = 0;
    write_offset_y += this->tallest_in_current_row; // go to next row
    this->tallest_in_current_row = 0; // reset this as we are in a new row
  } 

  if(bbox_height > tallest_in_current_row)
    tallest_in_current_row = bbox_height;


  D3D11_BOX region{};
  region.left = write_offset_x;
  region.right = write_offset_x + bbox_width;
  region.top = write_offset_y;
  region.bottom = write_offset_y + bbox_height;
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
                                    rendered_glyph_buffer,
                                    bbox_width * 4);

  write_offset_x += bbox_width;

  this->font_subregions.push_back(subregion);

  /* free the memory used to store the bitmap data */
  VirtualFree(rendered_glyph_buffer, bbox_width * bbox_height * 4, MEM_DECOMMIT | MEM_RELEASE);
  return this->font_subregions.size()-1; // index in the vector is the ID 
}

void FontRenderer::draw_string(const char* str, u32 len, Float2 translate, Float4 color, FontID font_id) {
  FontSubregion* font;
  font = this->get_font_by_id(font_id);

  if(!str) return;
  if(!font) return;

  /*TextLayoutParams*/
  float line_height = (f32)font->font->line_height; 
  float pen_x = 0;
  float pen_y = 0;

  for(int i = 0; i < len; ++i) {
    if(str[i] == '\n') {
      pen_y += line_height;
      pen_x = 0;
      continue;
    }

    if(str[i] == ' ') {
      pen_x += font->font->font_face->max_advance_width >> 6;
    }

    GlyphInfo* glyph = &font->font->glyph_info[str[i]];
    Float4 quad = this->generate_quad(str[i], font);
    Float4 uvs = this->calculate_uv(str[i], font);

    i32 glyph_height = glyph->y1 - glyph->y0;

    quad.x += pen_x + glyph->x_offset;
    quad.z += pen_x + glyph->x_offset;
    quad.y -= pen_y + (glyph_height - glyph->y_offset);
    quad.w -= pen_y + (glyph_height - glyph->y_offset);

    quad.x += translate.x;
    quad.z += translate.x;

    quad.y += translate.y;
    quad.w += translate.y;

    this->generate_vertices(quad, uvs, color);
    pen_x += glyph->advance.x;
    pen_y += glyph->advance.y;
  }
}

void FontRenderer::draw_string(std::string* str, Float2 translate, Float4 color, FontID font) {
  this->draw_string(str->c_str(), 
                    str->length(),
                    translate,
                    color,
                    font);

}

void FontRenderer::draw_string(std::string* str, TextStyle style) {
  this->draw_string(str->c_str(), 
                    str->length(), 
                    style.pos, 
                    style.color, 
                    style.font);
}

void FontRenderer::draw_string(std::string str, Float2 translate, Float4 color, FontID font) {
  this->draw_string(str.c_str(), 
                    str.length(), 
                    translate, 
                    color, 
                    font);
}

void FontRenderer::draw_string(std::string str, TextStyle style) {
  this->draw_string(str.c_str(), 
                    str.length(),
                    style.pos,
                    style.color,
                    style.font);

}

// generates a non-transformed quad that is simply the of the glyph
// will need to be transformed/layout'd 
Float4 FontRenderer::generate_quad(const char c, FontSubregion* font) {
  Float4 quad{};
  GlyphInfo glyph_info = font->font->glyph_info[c];
  quad.x = 0;
  quad.y = 0;
  // quad.x = 0.f;
  // quad.y = 0.f;
  quad.z = glyph_info.x1 - glyph_info.x0;
  quad.w = glyph_info.y1 - glyph_info.y0;
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

  u32 num_quads = this->next_free_vtx/4;
  this->font_idx_mem[this->next_free_idx + 0] = num_quads * 4 + 2;
  this->font_idx_mem[this->next_free_idx + 1] = num_quads * 4 + 3;
  this->font_idx_mem[this->next_free_idx + 2] = num_quads * 4 + 1;
  this->font_idx_mem[this->next_free_idx + 3] = num_quads * 4 + 2;
  this->font_idx_mem[this->next_free_idx + 4] = num_quads * 4 + 1;
  this->font_idx_mem[this->next_free_idx + 5] = num_quads * 4 + 0;

  this->next_free_idx += 6;
  this->next_free_vtx += 4;
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

  renderer->set_index_buffer(this->font_index_buffer.Get());

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
