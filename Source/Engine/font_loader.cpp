#include "font_loader.h"
#include "files.h"
#include "logging.h"
#include <cmath>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
FontLoader::FontLoader() { init(); }

FontLoader::~FontLoader() { shutdown(); }

bool FontLoader::is_loaded(std::string font_name) {
  return loaded_fonts.count(font_name) > 0;
}

void FontLoader::init() {
  FT_Error e = FT_Init_FreeType(&library);
  LOG_INFO("init freetype... {}", FT_Error_String(e));
}

void FontLoader::shutdown() {
  for (auto &[name, font_atlas] : loaded_fonts) {
    FT_Error e = FT_Done_Face(font_atlas.font_face);
    LOG_INFO("freeing {}: {}", name, FT_Error_String(e));

    // free the buffers
    if (font_atlas.atlas_buffer)
      delete font_atlas.atlas_buffer;
    if (font_atlas.glyph_info)
      delete font_atlas.glyph_info;
  }
  FT_Error e = FT_Done_FreeType(library);
  LOG_INFO("closing freetype... {}", FT_Error_String(e));
}

void FontLoader::list_loaded() {
  LOG_INFO("Loaded Fonts: ");
  for (auto &[name, font_atlas] : loaded_fonts)
    LOG_INFO("\t{:<30}: {:<30}", name, font_atlas.font_face->height / 64);
}

bool FontLoader::load_font(fs::path path,
                           u32 font_height,
                           u32 resolution,
                           u32 num_glyphs) {
  FT_Error fterror{};
  FT_Face font_face{};

  // font settings temp:

  // TODO probably want a range because first couple chars
  // are not renderable
  // end font settings

  if (!Files::file_exists(path)) {
    LOG_WARNING("loading: {}, error: file does not exist (check privs and that "
                "the path is correct!)",
                path.string());
    return false;
  }

  fterror = FT_New_Face(library, path.string().c_str(), 0, &font_face);
  if (fterror != FT_Err_Ok) {
    LOG_WARNING("error while loading {}:  {}",
                path.string(),
                FT_Error_String(fterror));
    return false;
  }

  // if this is a new font family
  // make new font atlas
  bool font_loaded_before = is_loaded(font_face->family_name);
  if (!font_loaded_before) {
    FontAtlas &atlas = loaded_fonts[font_face->family_name];
    atlas.font_face = font_face;

    // create the font
    {
      FT_Error check;

      // set the fonts size to the requested height
      // error = FT_Set_Pixel_Sizes(face_ref, 0, height_px);
      check = FT_Set_Char_Size(font_face,
                               0,
                               font_height << 6,
                               resolution,
                               resolution);

      // TODO improve the calculation here to a bit better,
      // currently this wastes a lot of space

      i32 max_size = (1 + (font_face->size->metrics.height >> 6)) *
                     ceilf(sqrtf(num_glyphs));
      atlas.width = 1;
      while (atlas.width < max_size) {
        atlas.width <<= 1; // increment in powers of 2
      }

      // Its a square texture I guess? The atlas probably wont be packed well
      // see TODO above.
      atlas.height = atlas.width;
      // LOG_INFO("creating font altas w/ dims = ({}, {})", width, height);

      // allocate enough space for the requested glyphs
      atlas.atlas_buffer = new u8[atlas.width * atlas.height];
      atlas.atlas_size = atlas.width * atlas.height;

      atlas.glyph_count = num_glyphs;
      atlas.glyph_info = new GlyphInfo[atlas.glyph_count];
      // setup pen
      //
      u32 freetype_load_mode =
          FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_NORMAL;
      i32 pen_x = 0;
      i32 pen_y = 0;

      // extract all the glyphs and put them into the atlas bitmap buffer
      for (int i = 0; i < num_glyphs; ++i) {
        FT_Load_Char(font_face, i, freetype_load_mode);
        FT_Bitmap *bitmap = &font_face->glyph->bitmap;

        // if we filled up this row, go to next one
        if (pen_x + bitmap->width >= atlas.width) {
          pen_x = 0;
          pen_y += (font_face->size->metrics.height >> 6) + 1;
        }

        // surely this can be re-written as a memcpy or something?
        for (i32 row = 0; row < bitmap->rows; ++row) {
          for (i32 col = 0; col < bitmap->width; ++col) {
            u32 x = (pen_x + col);
            u32 y = (pen_y + row);
            atlas.atlas_buffer[y * atlas.width + x] =
                bitmap->buffer[row * bitmap->pitch + col];
          }
        }

        atlas.glyph_info[i].x0 = pen_x;
        atlas.glyph_info[i].y0 = pen_y;
        atlas.glyph_info[i].x1 = pen_x + bitmap->width;
        atlas.glyph_info[i].y1 = pen_y + bitmap->rows;
        atlas.glyph_info[i].x_offset = font_face->glyph->bitmap_left;
        atlas.glyph_info[i].y_offset = font_face->glyph->bitmap_top;
        atlas.glyph_info[i].advance = font_face->glyph->advance.x >> 6;

        pen_x += bitmap->width + 1;
      }
    }

    LOG_INFO("loaded {}", font_face->family_name);
    return true;
  } else {
    // we saw this font before and loaded it successfully
    // free the temporarily loaded fontface object and return true
    FT_Done_Face(font_face);
    return true;
  }
}

void FontLoader::dump_atlases() {
  for (auto &[name, atlas] : loaded_fonts) {
    LOG_INFO("dumping atlas for {}", name);

    u8 *png_data = new u8[atlas.width * atlas.height * 4]{};
    if (atlas.atlas_buffer) {

      for (int i = 0; i < (atlas.width * atlas.height); ++i) {

        png_data[i * 4 + 0] |= atlas.atlas_buffer[i];
        png_data[i * 4 + 1] |= atlas.atlas_buffer[i];
        png_data[i * 4 + 2] |= atlas.atlas_buffer[i];
        png_data[i * 4 + 3] |= 0xff;
      }

      stbi_write_png(fmt::format("{}.png", name).c_str(),
                     atlas.width,
                     atlas.height,
                     4,
                     png_data,
                     atlas.width * 4);
      delete[] png_data;
    }
  }
}

u8 *FontLoader::get_atlas_texture(std::string family_name) {
  u8 *texture_data{};
  if (is_loaded(family_name)) {

    FontAtlas &atlas = loaded_fonts[family_name];
    // we need to put the font into RGBA format
    texture_data = new u8[atlas.width * atlas.height * 4]{};
    if (atlas.atlas_buffer) {
      for (int i = 0; i < (atlas.width * atlas.height); ++i) {
        texture_data[i * 4 + 0] = atlas.atlas_buffer[i];
        texture_data[i * 4 + 1] = atlas.atlas_buffer[i];
        texture_data[i * 4 + 2] = atlas.atlas_buffer[i];
        texture_data[i * 4 + 3] = 0xff;
      }
    }
  }
  return texture_data; // caller owns this allocation
}
FontAtlas *FontLoader::get(std::string family_name) {
  if (is_loaded(family_name))
    return &loaded_fonts[family_name];
  else
    return nullptr;
}
