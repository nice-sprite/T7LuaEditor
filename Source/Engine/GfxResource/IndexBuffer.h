//
// Created by coxtr on 11/25/2021.
//

#ifndef T7LUAEDITOR_INDEXBUFFER_H
#define T7LUAEDITOR_INDEXBUFFER_H
#include <d3d11_4.h>

HRESULT create_dynamic_index_buffer(ID3D11Device *device, ID3D11Buffer **ppBuffer, int numIndices);
HRESULT create_dynamic_index_buffer(ID3D11Device *device, ID3D11Buffer **ppBuffer, int *initialData, int numIndices);
void bind_dynamic_index_buffer(ID3D11DeviceContext *context, ID3D11Buffer *buf);
void update_dynamic_index_buffer(ID3D11DeviceContext *context, ID3D11Buffer *buf, int *indexData, size_t numIndices);
void append_dynamic_index_buffer(ID3D11DeviceContext *context, ID3D11Buffer *buf, int *indexData, size_t numIndices, size_t offset);

#endif //T7LUAEDITOR_INDEXBUFFER_H
