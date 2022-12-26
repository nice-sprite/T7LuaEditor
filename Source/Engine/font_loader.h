#pragma once
#include "../defines.h"
#include "files.h"
#include <freetype/freetype.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>

struct GlyphInfo {
  i32 x0, y0, x1, y1;
  i32 x_offset, y_offset;
  i32 advance;
};

struct FontAtlas {
  u8 *atlas_buffer;
  GlyphInfo *glyph_info;
  FT_Face font_face;
  u32 glyph_count;
  u32 atlas_size;
  u32 width;
  u32 height;
};

struct FontLoader {
  // data members
  FT_Library library;

  FontLoader();
  ~FontLoader();

private:
  // disable copying
  FontLoader operator=(const FontLoader &) = delete;
  FontLoader(const FontLoader &) = delete;

  std::unordered_map<std::string, FontAtlas> loaded_fonts{};

public:
  // initialize freetype
  void init();

  // free all loaded font faces and shutdown freetype
  void shutdown();

  // creates a texture glyph atlas
  // retrieve from loaded_fonts using the family name as the key
  bool
  load_font(fs::path path, u32 font_height, u32 resolution, u32 num_glyphs);

  // LOG_INFOS all the loaded font faces
  void list_loaded();

  void dump_atlases();

  bool is_loaded(std::string font_name);

  u8 *get_atlas_texture(std::string family_name);

  FontAtlas *get(std::string family_name);
};
