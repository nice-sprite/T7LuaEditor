#ifndef GPU_RESOURCES_H
#define GPU_RESOURCES_H

#include <d3d11_4.h>

constexpr auto CbufSlot_PerSceneConst = 0u;
constexpr auto CbufSlot_Transforms = 1u;

/* CONSTANT BUFFER FUNCTIONS */
void bind_constant_buffer(ID3D11DeviceContext *context,
                          int bufferSlot,
                          ID3D11Buffer *buffer);

HRESULT create_constant_buffer(ID3D11Device *device,
                               size_t bufferSizeInBytes,
                               ID3D11Buffer **ppBuffer);

HRESULT update_constant_buffer(ID3D11DeviceContext *context,
                               int bufferSlot,
                               void *newData,
                               size_t dataSize,
                               ID3D11Buffer *buffer);

HRESULT create_constant_buffer(ID3D11Device *device,
                               void *data,
                               size_t bufferSize,
                               ID3D11Buffer **ppBuffer);

/* VERTEX BUFFER FUNCTIONS */
HRESULT create_dynamic_vertex_buffer(ID3D11Device *device,
                                     ID3D11Buffer **ppBuffer,
                                     int reserveSize);

HRESULT create_dynamic_vertex_buffer(ID3D11Device *device,
                                     ID3D11Buffer **ppBuffer,
                                     void *initialData,
                                     int sizeInBytes);

void bind_dynamic_vertex_buffers(ID3D11DeviceContext *context,
                                 ID3D11Buffer **buf,
                                 UINT stride,
                                 UINT offset);

void update_dynamic_vertex_buffer(ID3D11DeviceContext *context,
                                  ID3D11Buffer *buf,
                                  void *vertexData,
                                  size_t sizeInBytes);

void append_dynamic_vertex_buffer(ID3D11DeviceContext *context,
                                  ID3D11Buffer *buf,
                                  void *vertexData,
                                  size_t sizeInBytes,
                                  size_t offsetInBytes);

/* INDEX BUFFER FUNCTIONS */
HRESULT create_dynamic_index_buffer(ID3D11Device *device,
                                    ID3D11Buffer **ppBuffer,
                                    int numIndices);

HRESULT create_dynamic_index_buffer(ID3D11Device *device,
                                    ID3D11Buffer **ppBuffer,
                                    int *initialData,
                                    int numIndices);

void bind_dynamic_index_buffer(ID3D11DeviceContext *context, ID3D11Buffer *buf);

void update_dynamic_index_buffer(ID3D11DeviceContext *context,
                                 ID3D11Buffer *buf,
                                 int *indexData,
                                 size_t numIndices);

void append_dynamic_index_buffer(ID3D11DeviceContext *context,
                                 ID3D11Buffer *buf,
                                 int *indexData,
                                 size_t numIndices,
                                 size_t offset);

/* TEXTURE FUNCTIONS */
void alloc_texture_atlas(int slot, ID3D11ShaderResourceView **outTextureHandle);

#endif
