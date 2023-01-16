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

struct Font {
  GlyphInfo *glyph_info; // only need this for rendering...
  FT_Face font_face;
  u32 glyph_count; // put this in glyphinfo
  u32 width;
  u32 height;

  // loaded font face
  Font(FT_Face font_face)
      : font_face{font_face} {}
};

// Wrapper around Freetype
struct FontLoader {
  FT_Library library;

  FontLoader();
  ~FontLoader();

private:
  // disable copying
  FontLoader operator=(const FontLoader &) = delete;
  FontLoader(const FontLoader &) = delete;

  // init freetype library instance
  void init();

  // free all loaded font faces and shutdown freetype
  void shutdown();

public:
  // initialize freetype

  // loads a ttf font
  Font* load_font(fs::path path, u32 font_height, u32 resolution, u32 num_glyphs);

  // u8 *get_atlas_texture(std::string family_name);

  // FontAtlas *get_ptr(std::string family_name);
};
