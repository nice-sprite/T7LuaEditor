//
// Created by coxtr on 12/14/2021.
//

#ifndef T7LUAEDITOR_SCENESYSTEM_H
#define T7LUAEDITOR_SCENESYSTEM_H

#include <vector>
#include <string>


using ShaderId = uint16_t;
using FontId = uint16_t;
using Image = uint16_t;
using Text = uint16_t;

struct ImageDef {
    const char name[64];
    float left, right, top, bottom, r, g, b, alpha;
    ShaderId shaderId;
};

struct TextDef {
    FontId fontId;
    ShaderId shaderId;
    std::string contents;
    float left, right, top, bottom, r, g, b, alpha;
};



class SceneSystem {
    std::vector<ImageDef> images_;
    std::vector<TextDef> text_;

public:

    SceneSystem();

    Image AddImage(ImageDef);

    Text AddText(TextDef);


};


std::vector<ImageDef> Layout(std::vector<ImageDef> const&  images) {
    for(int i = 0; i < images.size(); ++i) {

    }
    return {};
}


#endif //T7LUAEDITOR_SCENESYSTEM_H
