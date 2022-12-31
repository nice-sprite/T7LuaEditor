#include "font_loader.h"
#include "files.h"
#include "logging.h"
#include <cmath>

FontLoader::FontLoader() { init(); }

FontLoader::~FontLoader() { shutdown(); }

void FontLoader::init() {
  FT_Error e = FT_Init_FreeType(&library);
  LOG_INFO("init freetype... {}", FT_Error_String(e));
}

void FontLoader::shutdown() {
  FT_Error e = FT_Done_FreeType(library);
  LOG_INFO("closing freetype... {}", FT_Error_String(e));
}

Font *FontLoader::load_font(fs::path path,
                            u32 font_height,
                            u32 resolution,
                            u32 num_glyphs) {
  FT_Error fterror{};
  FT_Face font_face{};
  Font *font_data;

  if (!Files::file_exists(path)) {
    LOG_WARNING("loading: {}, error: file does not exist (check privs and that "
                "the path is correct!)",
                path.string());
    return nullptr;
  }

  fterror = FT_New_Face(library, path.string().c_str(), 0, &font_face);
  if (fterror != FT_Err_Ok) {
    LOG_WARNING("error while loading {}:  {}",
                path.string(),
                FT_Error_String(fterror));
    return nullptr;
  }

  font_data = new Font(font_face);
  // if this is a new font family
  // make new font atlas

  // create the font

  FT_Error check;

  // set the fonts size to the requested height
  // error = FT_Set_Pixel_Sizes(face_ref, 0, height_px);
  check =
      FT_Set_Char_Size(font_face, 0, font_height << 6, resolution, resolution);

  // TODO improve the calculation here to a bit better,
  // currently this wastes a lot of space

  i32 max_size =
      (1 + (font_face->size->metrics.height >> 6)) * ceilf(sqrtf(num_glyphs));

  font_data->width = 1;
  while (font_data->width < max_size) {
    font_data->width <<= 1; // increment in powers of 2
  }

  // Its a square texture I guess? The font_data->probably wont be packed well
  // see TODO above.
  font_data->height = font_data->width;
  // LOG_INFO("creating font altas w/ dims = ({}, {})", width, height);

  // allocate enough space for the requested glyphs

  font_data->glyph_count = num_glyphs;
  font_data->glyph_info = new GlyphInfo[font_data->glyph_count];
  u32 freetype_load_mode =
      FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_NORMAL;
  // setup pen
  i32 pen_x = 0;
  i32 pen_y = 0;

  // extract all the glyphs and put them into the font_data->bitmap buffer
  for (int i = 0; i < num_glyphs; ++i) {
    FT_Load_Char(font_face, i, freetype_load_mode);
    FT_Bitmap *bitmap = &font_face->glyph->bitmap;

    // if we filled up this row, go to next one
    if (pen_x + bitmap->width >= font_data->width) {
      pen_x = 0;
      pen_y += (font_face->size->metrics.height >> 6) + 1;
    }

    // surely this can be re-written as a memcpy or something?
    //      for (i32 row = 0; row < bitmap->rows; ++row) {
    //        for (i32 col = 0; col < bitmap->width; ++col) {
    //          u32 x = (pen_x + col);
    //          u32 y = (pen_y + row);
    //          font_data->buffer[y * font_data->width + x] =
    //              bitmap->buffer[row * bitmap->pitch + col];
    //        }
    //      }

    font_data->glyph_info[i].x0 = pen_x;
    font_data->glyph_info[i].y0 = pen_y;
    font_data->glyph_info[i].x1 = pen_x + bitmap->width;
    font_data->glyph_info[i].y1 = pen_y + bitmap->rows;
    font_data->glyph_info[i].x_offset = font_face->glyph->bitmap_left;
    font_data->glyph_info[i].y_offset = font_face->glyph->bitmap_top;
    font_data->glyph_info[i].advance = font_face->glyph->advance.x >> 6;

    pen_x += bitmap->width + 1;
  }

  // get a much better approximation of the needed height
  font_data->height = font_data->glyph_info[font_data->glyph_count - 1].y1 +
                      (font_face->size->metrics.height >> 6) + 1;

  return font_data;
}

// u8 *FontLoader::get_atlas_texture(std::string family_name) {
//   u8 *texture_data{};
//   if (is_loaded(family_name)) {
//     FontAtlas &atlas = loaded_fonts[family_name];
//     // we need to put the font into RGBA format
//     texture_data = new u8[atlas.width * atlas.height * 4]{};
//     if (atlas.atlas_buffer) {
//       for (int i = 0; i < (atlas.width * atlas.height); ++i) {
//         texture_data[i * 4 + 0] = atlas.atlas_buffer[i];
//         texture_data[i * 4 + 1] = atlas.atlas_buffer[i];
//         texture_data[i * 4 + 2] = atlas.atlas_buffer[i];
//         texture_data[i * 4 + 3] = 0xff;
//       }
//     }
//   }
//   return texture_data; // caller owns this allocation
// }

// FontAtlas *FontLoader::get_ptr(std::string family_name) {
//   if (is_loaded(family_name))
//     return &loaded_fonts[family_name];
//   else
//     return nullptr;
// }
