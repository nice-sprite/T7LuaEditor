#pragma once
#include "../defines.h"
#include "files.h"
#include "math.h"
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftmodapi.h>
#include FT_FREETYPE_H
#include <ft2build.h>
#include <unordered_map>

static constexpr u32 Start_Glyph = 32; // the first 32 are just control codes;
struct GlyphInfo {
  i32 x0, y0, x1, y1;
  i32 x_offset, y_offset;
  Float2 advance;
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

  Float2 calculate_atlas_dimensions(FT_Face font_face, i32 num_glyphs = 128, i32 max_width = 2048);

public:
  // initialize freetype

  // loads a ttf font
  Font* load_font(fs::path path, u32 font_height, u32 resolution, u32 num_glyphs);

};
