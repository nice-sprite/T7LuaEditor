//
// Created by coxtr on 12/22/2021.
//

#ifndef T7LUAEDITOR_TEXTUREMANAGER_H
#define T7LUAEDITOR_TEXTUREMANAGER_H

#include "Texture.h"
#include <vector>
#include <string>
#include <thread>
//

class TextureManager {
private:
    std::thread* workerThread;
    static const unsigned int MaxTextures = 20000;
    static const unsigned int TexturesPerBatch = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
    using TextureID  = size_t;
    // std::array<ID3D11ShaderResourceView, MaxTextures> textureViews_;
    // std::array<ID3D11Resource, MaxTextures> resourceViews_;
    ID3D11Device* device;
public:
    // we need a device to be able to create textures on GPU
    TextureManager(ID3D11Device* device);
    TextureID RequestLoad(std::string path);
    TextureID RequestLoad(std::wstring path);

    // load multiple textures at once
    std::vector<TextureID> BatchLoad(std::vector<std::string> paths);
    std::vector<TextureID> BatchLoad(std::vector<std::wstring> paths);

    Texture& GetTexture(TextureID texId);

};


#endif //T7LUAEDITOR_TEXTUREMANAGER_H
