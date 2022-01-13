//
// Created by coxtr on 11/25/2021.
//

#ifndef T7LUAEDITOR_INDEXBUFFER_H
#define T7LUAEDITOR_INDEXBUFFER_H
#include "../../t7pch.h"

HRESULT CreateDynamicIndexBuffer(ID3D11Device *device, ID3D11Buffer **ppBuffer, int numIndices);
HRESULT CreateDynamicIndexBuffer(ID3D11Device *device, ID3D11Buffer **ppBuffer, int *initialData, int numIndices);
void BindDynamicIndexBuffer(ID3D11DeviceContext *context, ID3D11Buffer *buf);
void UpdateDynamicIndexBuffer(ID3D11DeviceContext *context, ID3D11Buffer *buf, int *indexData, size_t numIndices);
void AppendDynamicIndexBuffer(ID3D11DeviceContext *context, ID3D11Buffer *buf, int *indexData, size_t numIndices, size_t offset);

#endif //T7LUAEDITOR_INDEXBUFFER_H