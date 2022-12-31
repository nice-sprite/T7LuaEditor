#pragma once
#include "font_loader.h"
#include "math.h"
#include "renderer.h"
#include <string>

struct FontManager {
  FontManager();
  FontLoader loader;

  std::vector<Font *> fonts;

  // the UV offset into the texture atlas where this font starts
  std::vector<Float2> offset;

  // returns the index of the font if it was loaded, -1 otherwise
  i32 register_font(std::string path, u32 height);
};

struct FontRenderer {
  Texture2D atlas; // use a big ol' texture to store multiple fonts inside of

  FontManager manager;

  FontRenderer(Renderer &renderer);

  bool init_atlas_texture(Renderer &renderer);

  i32 load_font(Renderer &renderer, std::string path, u32 height);
};