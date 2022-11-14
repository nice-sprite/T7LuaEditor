//
// Created by coxtr on 12/22/2021.
//

#include "AssetView.h"
// #include <entt/entt.hpp>

void DrawAssetViewDirectorySelection(AssetViewParams &assets) {
    auto assetTypeFilter = {
            ".png",
            ".jpg",
            ".tiff",
            ".tif",
            ".bmp"
    };
    auto ExtensionFilter = [&assetTypeFilter](std::filesystem::path const &path) -> bool {
        for (auto &type: assetTypeFilter) {
            if (path.extension() == type) return true;
        }
        return false;
    };
    if (ImGui::Begin("Asset Picker", nullptr, ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMenuBar()) {
            if (ImGui::Button("+Add Directory..."))
                ImGuiFileDialog::Instance()->OpenModal("AssetDirKey", "Choose Directory", nullptr, ".");
            if (ImGuiFileDialog::Instance()->Display("AssetDirKey")) {
                if (ImGuiFileDialog::Instance()->IsOk()) {
                    assets.mapDirectory = ImGuiFileDialog::Instance()->GetCurrentPath();
                    std::filesystem::directory_iterator dirIt(assets.mapDirectory);
                    for (auto const &path: dirIt) {
                        if (ExtensionFilter(path))
                            InsertAssetPath(assets.files, path.path());
                    }
                    std::thread textureLoadThread([&assets]() -> void {
                        CoInitialize(nullptr);
                        for (auto const &texPath: assets.files) {
                            assets.thumbnailTextures.push_back(
                                    LoadTexture(assets.device, nullptr, texPath.c_str())
                            );
                        }
                    });
                    textureLoadThread.detach();
                }
                ImGuiFileDialog::Instance()->Close();
            }
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}
/**/
/**/
void DrawAssets(AssetViewParams &assets) {
    static char filterBuff[512] = {0};
    int i = 0;
    if (ImGui::Begin("Asset Picker")) {
        const auto viewWidth = ImGui::GetContentRegionAvailWidth();
        auto currentRowWidth = viewWidth; // skip the first line
        int maxPerRow = (int) viewWidth / assets.previewSize_;
/**/
/**/
        ImGui::SliderInt("preview size", &assets.previewSize_, 0, 512);
        if (ImGui::TreeNodeEx(assets.mapDirectory.c_str(), ImGuiTreeNodeFlags_OpenOnArrow)) {
/**/
            ImGui::InputTextWithHint("Filter", "Search...", filterBuff, 512);
/**/
            ImGui::Columns(maxPerRow);
            ImGui::Separator();
/**/
            for (auto const &thumbnail: assets.thumbnailTextures) {
                if (PathContains(assets.files.at(i), std::string(filterBuff))) {
                    auto thumbnailArea = CalcThumbnailDimensions(thumbnail, assets.previewSize_);
/**/
                    auto buttonPressed = ImGui::ImageButton((void *) thumbnail.view_.Get(),
                                                            ImVec2((float) assets.previewSize_,
                                                                   (float) assets.previewSize_));
                    ImGui::Text("%ls", assets.files.at(i).filename().c_str());
                    ImGui::NextColumn();
/**/
                    if (buttonPressed) {
                        // add ent to scene
/**/
/**/
                    }
                }

                i++;
            }
            ImGui::Columns(1);
            ImGui::TreePop();
        }
    }
    ImGui::End();

}

// // fits the image to 128x128 while maintaining aspect ratio
// ImVec2 CalcThumbnailDimensions(Texture const& thumbnail, int previewSize)
// {
//     float aspectRatio = thumbnail.width_ / thumbnail.height_;
//     float scaleFactor = previewSize / thumbnail.height_;

//     if (aspectRatio > 1.f)
//     {
//         scaleFactor = previewSize / thumbnail.width_;
//     }

//     return {thumbnail.width_ * scaleFactor, thumbnail.height_ * scaleFactor};
// }


// bool PathContains(std::filesystem::path const& path, std::string const& search)
// {
//     if (search.empty()) return true;
//     auto lowerPath = ToLower(path.filename().string());
//     auto lowerSearch = ToLower(search);
//     int i = 0;
//     while (i < lowerSearch.length() && lowerPath[i] == lowerSearch[i]) ++i;
//     return i == search.length();
// }

// std::string ToLower(std::string const& str)
// {
//     std::string ret;
//     for (auto const& c: str) ret.push_back((char) ::tolower(c));
//     return ret;
// }

// void InsertAssetPath(Files& files, std::filesystem::path const& path)
// {
//     if (std::find(files.begin(), files.end(), path) == files.end())
//     {
//         files.push_back(path);
//     }
// }
