#include "files.h"
#include "logging.h"
#include <freetype/freetype.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>

struct FontLoader {
  // data members
  FT_Library library;
  std::unordered_map<std::string, FT_Face> loaded_fonts{};

  FontLoader();

private:
  // disable copying
  FontLoader operator=(const FontLoader &) = delete;
  FontLoader(const FontLoader &) = delete;

public:
  // initialize freetype
  void init();

  // free all loaded font faces and shutdown freetype
  void shutdown();

  // loads a font face and stores it in loaded_fonts
  void load_font(std::string path);
  void load_font(fs::path path);

  // LOG_INFOS all the loaded font faces
  void list_loaded();

  bool set_font_size_pixels(std::string fontname, int size_pts);
  // void draw(std::string font, std::string text);
};
