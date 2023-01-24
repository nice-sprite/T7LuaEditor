#include "font_loader.h"
#include "files.h"
#include <freetype/ftsystem.h>
#include "logging.h"
#include <cmath>
#include <Windows.h>
#include <unordered_map>
#include <map>

FontLoader::FontLoader() { init(); }

FontLoader::~FontLoader() { shutdown(); }

void FontLoader::init() {
  FT_Error e = FT_Init_FreeType(&library);
  LOG_INFO("init freetype... {}", FT_Error_String(e));

  FT_Module sdf_module = FT_Get_Module(library, "sdf");
  FT_Module bsdf_module = FT_Get_Module(library, "bsdf");
  if(sdf_module == 0) {
    LOG_WARNING("no sdf motherfucker");
  }

  if(bsdf_module == 0) {
    LOG_WARNING("no bsdf motherfucker");
  }
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
  FT_Error check;
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
  
  // set the fonts size to the requested height
  // error = FT_Set_Pixel_Sizes(face_ref, 0, height_px);
  check = FT_Set_Char_Size(font_face, 0, font_height << 6, resolution, resolution);


  Float2 atlas_dims = this->calculate_atlas_dimensions(font_face, num_glyphs);
  LOG_INFO("atlas dimensions should be: {}x{}", atlas_dims.x, atlas_dims.y);

  // Its a square texture I guess? The font_data->probably wont be packed well
  // see TODO above.
  font_data->width = atlas_dims.x;
  font_data->height = atlas_dims.y;
  // LOG_INFO("creating font altas w/ dims = ({}, {})", width, height);

  // allocate enough space for the requested glyphs
  font_data->glyph_count = num_glyphs;
  font_data->glyph_info = new GlyphInfo[font_data->glyph_count];
  u32 freetype_load_mode = FT_LOAD_DEFAULT;
  // setup pen
  i32 pen_x = 0;
  i32 pen_y = 0;

  // extract all the glyphs and put them into the font_data->bitmap buffer
  for (int i = Start_Glyph; i < num_glyphs; ++i) {
    FT_UInt glyph_index = FT_Get_Char_Index(font_face, i);
    FT_Set_Pixel_Sizes(font_face, 0, (FT_UInt)24);
    FT_Load_Glyph(font_face, glyph_index, freetype_load_mode);
    FT_Bitmap *bitmap = &font_face->glyph->bitmap;

    // if we filled up this row, go to next one
    if (pen_x + bitmap->width >= font_data->width) {
      pen_x = 0;
      pen_y += (font_face->size->metrics.height >> 6) + 1;
    }

    font_data->glyph_info[i].x0 = pen_x;
    font_data->glyph_info[i].y0 = pen_y;
    font_data->glyph_info[i].x1 = pen_x + bitmap->width;
    font_data->glyph_info[i].y1 = pen_y + bitmap->rows;
    font_data->glyph_info[i].x_offset = font_face->glyph->bitmap_left;
    font_data->glyph_info[i].y_offset = font_face->glyph->bitmap_top;
    font_data->glyph_info[i].advance.x = font_face->glyph->advance.x >> 6;
    font_data->glyph_info[i].advance.y = font_face->glyph->advance.y >> 6;

    pen_x += bitmap->width + 1;
  }

  // get a much better approximation of the needed height
  font_data->height = font_data->glyph_info[font_data->glyph_count - 1].y1 +
                      (font_face->size->metrics.height >> 6) + 1;

  return font_data;
}

// max_width makes sure that the total width of the GPU buffer isnt used
// by 1 font atlas. 
// this may not actually matter, but I think we get better packing this way
Float2 FontLoader::calculate_atlas_dimensions(
    FT_Face font_face, 
    i32 num_glyphs,
    i32 max_width) {
  Float2 dims{};
  // freetype really sucks, so the only way I can 
  // imagine getting the actual correct total width and height
  // for the atlas texture is by rendering it all twice!!! 
  // Everything is terrible and needs re-written
  for(int i = Start_Glyph; i < num_glyphs; ++i) {
    FT_Set_Pixel_Sizes(font_face, 0, (FT_UInt)24);
    FT_UInt glyph_index = FT_Get_Char_Index(font_face, i);
    FT_Error check = FT_Load_Glyph(font_face, i, FT_LOAD_DEFAULT);
    if(check != FT_Err_Ok)
      LOG_INFO("failed to load the glyph: {}", FT_Error_String(check));

    FT_Error check3 = FT_Render_Glyph(font_face->glyph, FT_RENDER_MODE_NORMAL);
    //FT_Error check2 = FT_Render_Glyph(font_face->glyph, FT_RENDER_MODE_SDF);

    if(dims.x + font_face->glyph->bitmap.width > max_width) {
      // new row;
      dims.x = 0;
      dims.y += font_face->glyph->metrics.height >> 6;
    }

    dims.x += font_face->glyph->bitmap.width;
  }
  return dims;
}

