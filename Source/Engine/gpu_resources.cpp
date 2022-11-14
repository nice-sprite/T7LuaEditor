#include "gpu_resources.h"
#include <windows.h>

/* CONSTANT BUFFER FUNCTIONS */
void bind_constant_buffer(ID3D11DeviceContext *context, int bufferSlot,
                          ID3D11Buffer *buffer) {
  context->VSSetConstantBuffers(bufferSlot, 1, &buffer);
}

HRESULT create_constant_buffer(ID3D11Device *device, size_t bufferSizeInBytes,
                               ID3D11Buffer **ppBuffer) {
  D3D11_BUFFER_DESC bd;
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.ByteWidth = bufferSizeInBytes;
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;
  HRESULT res = device->CreateBuffer(&bd, nullptr, ppBuffer);
  return res;
}

HRESULT update_constant_buffer(ID3D11DeviceContext *context, int bufferSlot,
                               void *newData, size_t dataSize,
                               ID3D11Buffer *buffer) {
  D3D11_MAPPED_SUBRESOURCE mr;
  HRESULT res =
      context->Map(buffer, bufferSlot, D3D11_MAP_WRITE_DISCARD, 0, &mr);
  memcpy(mr.pData, newData, dataSize);
  context->Unmap(buffer, bufferSlot);
  return res;
}

HRESULT create_constant_buffer(ID3D11Device *device, void *data,
                               size_t bufferSize, ID3D11Buffer **ppBuffer) {
  D3D11_BUFFER_DESC bd{};
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.ByteWidth = bufferSize;
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;
  D3D11_MAPPED_SUBRESOURCE rm;
  rm.pData = data;
  rm.DepthPitch = 0;
  rm.RowPitch = 0;
  HRESULT res =
      device->CreateBuffer(&bd, (const D3D11_SUBRESOURCE_DATA *)&rm, ppBuffer);
  return res;
}

/* INDEX BUFFER FUNCTIONS */
HRESULT create_dynamic_index_buffer(ID3D11Device *device,
                                    ID3D11Buffer **ppBuffer, int numIndices) {
  D3D11_BUFFER_DESC bd{};
  bd.ByteWidth = sizeof(int) * numIndices;
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;

  auto hres = device->CreateBuffer(&bd, nullptr, ppBuffer);
  return hres;
}

HRESULT create_dynamic_index_buffer(ID3D11Device *device,
                                    ID3D11Buffer **ppBuffer, int *initialData,
                                    int numIndices) {
  D3D11_BUFFER_DESC bd{};
  bd.ByteWidth = sizeof(int) * numIndices;
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA data{};
  data.pSysMem = initialData;
  data.SysMemPitch = 0;
  data.SysMemSlicePitch = 0;

  auto hres = device->CreateBuffer(&bd, &data, ppBuffer);
  return hres;
}

void bind_dynamic_index_buffer(ID3D11DeviceContext *context,
                               ID3D11Buffer *buf) {
  context->IASetIndexBuffer(buf, DXGI_FORMAT_R32_UINT, 0);
}

void update_dynamic_index_buffer(ID3D11DeviceContext *context,
                                 ID3D11Buffer *buf, int *indexData,
                                 size_t numIndices) {
  D3D11_MAPPED_SUBRESOURCE msr{};
  context->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
  memcpy(msr.pData, indexData, numIndices * sizeof(int));
  context->Unmap(buf, 0);
}

void append_dynamic_index_buffer(ID3D11DeviceContext *context,
                                 ID3D11Buffer *buf, int *indexData,
                                 size_t numIndices, size_t offset) {
  D3D11_MAPPED_SUBRESOURCE msr{};
  context->Map(buf, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0,
               &msr); // we are telling the device driver we won't overwrite
                      // data, only add
  int *gpuIndices = (int *)msr.pData;
  for (int i = 0; i < numIndices; ++i) {
    gpuIndices[i + offset] = indexData[i];
  }
  context->Unmap(buf, 0);
}

void bind_dynamic_index_buffer(ID3D11DeviceContext *context, ID3D11Buffer *buf,
                               int *indexData, size_t numIndices) {

  D3D11_MAPPED_SUBRESOURCE msr{};
  context->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0,
               &msr); // we are telling the device driver we won't overwrite
                      // data, only add

  int *gpuIndices = (int *)msr.pData;

  for (int i = 0; i < numIndices; ++i) {
    gpuIndices[i] = indexData[i];
  }
  context->Unmap(buf, 0);
}

/* VERTEX BUFFER FUNCTIONS */
HRESULT create_dynamic_vertex_buffer(ID3D11Device *device,
                                     ID3D11Buffer **ppBuffer, int reserveSize) {
  D3D11_BUFFER_DESC bd{};
  bd.ByteWidth = reserveSize;
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;
  auto hres = device->CreateBuffer(&bd, nullptr, ppBuffer);
  return hres;
}

HRESULT
create_dynamic_vertex_buffer(ID3D11Device *device, ID3D11Buffer **ppBuffer,
                             void *initialData, int sizeInBytes) {
  D3D11_BUFFER_DESC bd{};
  bd.ByteWidth = sizeInBytes;
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA data{};
  data.pSysMem = initialData;
  data.SysMemPitch = 0;
  data.SysMemSlicePitch = 0;

  auto hres = device->CreateBuffer(&bd, &data, ppBuffer);
  return hres;
}

void bind_dynamic_vertex_buffers(ID3D11DeviceContext *context,
                                 ID3D11Buffer **buf, UINT stride, UINT offset) {
  context->IASetVertexBuffers(0, 1, buf, &stride, &offset);
}

// updates the entire vertex buffer, discards previous contents
void update_dynamic_vertex_buffer(ID3D11DeviceContext *context,
                                  ID3D11Buffer *buf, void *vertexData,
                                  size_t sizeInBytes) {
  D3D11_MAPPED_SUBRESOURCE msr{};
  context->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
  memcpy(msr.pData, vertexData, sizeInBytes);
  context->Unmap(buf, 0);
}

void append_dynamic_vertex_buffer(ID3D11DeviceContext *context,
                                  ID3D11Buffer *buf, void *vertexData,
                                  size_t sizeInBytes, size_t offsetInBytes) {
  D3D11_MAPPED_SUBRESOURCE msr{};
  context->Map(buf, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &msr);
  // we are telling the device driver we won't overwrite data, only add
  unsigned char *beginData = (unsigned char *)msr.pData + offsetInBytes;
  memcpy((void *)beginData, vertexData, sizeInBytes);
  context->Unmap(buf, 0);
}
