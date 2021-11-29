//
// Created by coxtr on 11/22/2021.
//

#ifndef T7LUAEDITOR_VERTEXSHADER_H
#define T7LUAEDITOR_VERTEXSHADER_H
#include "Bindable.h"
#include "ShaderUtil.h"
#include <string>
class VertexShader : public Bindable {
public:
    VertexShader(Gfx& gfx, const wchar_t *filepath);
    void Bind(Gfx& gfx) override;
    ID3DBlob* GetBlob() const noexcept;
private:
    wrl::ComPtr<ID3D11VertexShader> vertexShader;
    wrl::ComPtr<ID3DBlob> bytecodeBlob;

};


#endif //T7LUAEDITOR_VERTEXSHADER_H
