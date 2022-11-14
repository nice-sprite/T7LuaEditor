//
// Created by coxtr on 12/22/2021.
//

#ifndef T7LUAEDITOR_ASSETVIEW_H
#define T7LUAEDITOR_ASSETVIEW_H

//#include "../../../3rdparty/ImGuiFileDialog/ImGuiFileDialog.h"
// #include "../../../Engine/Texture.h"

/*
 * AssetView
 * Usage:
 * Select a map folder -> get all images in assets folder
 */
using Files = std::vector<std::filesystem::path>;


struct AssetViewParams {
    ID3D11Device *device; // for creating thumbnail texture resources
    std::string mapDirectory;
    std::vector <Texture> thumbnailTextures;
    Files files;
    int previewSize_{128};
};


// if the path already exists, do not add it
void InsertAssetPath(Files &files, std::filesystem::path const &path);

ImVec2 CalcThumbnailDimensions(Texture const &thumbnail, int previewSize);

void DrawAssets(AssetViewParams &assets);

void DrawAssetViewDirectorySelection(AssetViewParams &assets);

bool PathContains(std::filesystem::path const &path, std::string const &search);

std::string ToLower(std::string const &str);

#endif //T7LUAEDITOR_ASSETVIEW_H
