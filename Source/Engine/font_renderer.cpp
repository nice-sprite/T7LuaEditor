#include "font_renderer.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

FontManager::FontManager()
    : loader() {}

i32 FontManager::register_font(std::string path, u32 height) {
  LOG_INFO("loading {} for render", path);
  Font *font = loader.load_font(fs::path(path), height, 110, 128);
  if (font) {
    fonts.push_back(font);
    // offset.push_back()->width
    return fonts.size() - 1;
  } else {
    LOG_WARNING("{} could not be loaded.\n", path);
    return -1;
  }
}

FontRenderer::FontRenderer(Renderer &renderer)
    : manager() {
  //
  init_atlas_texture(renderer);
}

bool FontRenderer::init_atlas_texture(Renderer &renderer) {
  TextureParams texdef{};

  texdef.desired_width = ResourceLimits::MaxTextureWidth >> 2;
  texdef.desired_height = ResourceLimits::MaxTextureHeight >> 2;
  texdef.format = TextureFormats::RGBA_8;
  texdef.usage = D3D11_USAGE_DYNAMIC;
  texdef.cpu_flags = D3D11_CPU_ACCESS_WRITE;

  bool check = renderer.create_texture(texdef, atlas);
  if (check) {
    LOG_INFO("Created atlas texture for fonts");
  } else {
    LOG_WARNING("Failed to create texture atlas");
  }

  return check;
}

i32 FontRenderer::load_font(Renderer &renderer, std::string path, u32 height) {
  i32 fontid = manager.register_font(path, height);
  Font *font = manager.fonts[fontid];

  u8 *rendered_glyphs = new u8[font->width * font->height * 4];
  u32 freetype_load_mode =
      FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_NORMAL;

  i32 pen_x = 0, pen_y = 0;
  for (int i = 0; i < font->glyph_count; ++i) {
    FT_Error check = FT_Load_Char(font->font_face, i, freetype_load_mode);
    if(check != FT_Err_Ok)
      LOG_WARNING("failed to load the glyph");
    FT_Bitmap *bitmap = &font->font_face->glyph->bitmap;
    LOG_INFO("bitmap pixel mode: {}", bitmap->pixel_mode);

    if (pen_x + bitmap->width >= font->width) {
      pen_x = 0;
      pen_y += (font->font_face->size->metrics.height >> 6) + 1;
    }

    for (i32 row = 0; row < bitmap->rows; ++row) {
      for (i32 col = 0; col < bitmap->width; ++col) {
        u32 x = (pen_x + col);
        u32 y = (pen_y + row);
        rendered_glyphs[ (y * font->width + x) * 4 + 0] = bitmap->buffer[row * bitmap->pitch + col];
        rendered_glyphs[ (y * font->width + x) * 4 + 1] = bitmap->buffer[row * bitmap->pitch + col];
        rendered_glyphs[ (y * font->width + x) * 4 + 2] = bitmap->buffer[row * bitmap->pitch + col];
        rendered_glyphs[ (y * font->width + x) * 4 + 3] = 0xff;
      }
    }
    pen_x += bitmap->width + 1;
  }

  stbi_write_png("debug.png", font->width, font->height, 4, rendered_glyphs, font->width * 4);

  renderer.update_texture(atlas, [=](u8 *buffer, u32 row_pitch) {
    LOG_INFO("row_pitch = {}", row_pitch);
    u8 *rg = rendered_glyphs;
    for (int row = 0; row < font->height; ++row) {
      memcpy(buffer, rg, font->width * 4);
      buffer += row_pitch;
      rg += font->width * 4;
    }
  });

  delete[] rendered_glyphs; // free the temp data
  return fontid;
}