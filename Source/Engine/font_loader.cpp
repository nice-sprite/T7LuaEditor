#include "font_loader.h"

void FontLoader::FontLoader() { init(); }
void FontLoader::init() {
  FT_Error e = FT_Init_FreeType(&library);
  LOG_INFO("init freetype... {}", FT_Error_String(e));
}

void FontLoader::shutdown() {
  for (auto [name, face] : loaded_fonts) {
    FT_Error e = FT_Done_Face(face);
    LOG_INFO("freeing {}: {}", name, FT_Error_String(e));
  }
  FT_Error e = FT_Done_FreeType(library);
  LOG_INFO("closing freetype... {}", FT_Error_String(e));
}

void FontLoader::list_loaded() {
  LOG_INFO("Loaded Fonts: ");
  for (auto [name, face] : loaded_fonts)
    LOG_INFO("\t{:<30}: {:<30}", name, face->height / 64);
}

void FontLoader::load_font(std::string path) {
  FT_Error fterror{};
  FT_Face fontface{};
  fterror = FT_New_Face(library, path.c_str(), 0, &fontface);

  if (fterror == FT_Err_Ok) {
    LOG_INFO("loaded {}", fontface->family_name);
    if (!loaded_fonts[fontface->family_name]) {
      loaded_fonts[fontface->family_name] = fontface;
    }
  } else {
    LOG_WARNING("error while loading {}:  {}", path, FT_Error_String(fterror));
  }
}

bool FontLoader::set_font_size_pixels(std::string fontname, int size_px) {
  if (loaded_fonts[fontname]) {
    auto face = loaded_fonts[fontname];
    FT_Error e = FT_Set_Pixel_Sizes(face, 0, 16);

    return e == FT_Err_Ok;
  } else {
    return false;
  }
}

// void FontLoader::draw(std::string font, std::string text) {
//   FT_GlyphSlot slot;
//   FT_Face face = loaded_fonts[font];
//   if (face) {
//     slot = face->glyph;
//     for (int i = 0; i < text.size(); ++i) {
//       FT_UInt glyph_index;
//       glyph_index = FT_Get_Char_Index(face, text[i]);
//       FT_Error e = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
//       if (e == FT_Err_Ok) {
//         e = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
//         if (e == FT_Err_Ok) {
//         } else {
//           LOG_INFO("error FT_Render_Glyph: {}", FT_Error_String(e));
//         }
//       } else {
//         LOG_INFO("error FT_Load_Glyph: {}", FT_Error_String(e));
//       }
//     }
//   }
// }
