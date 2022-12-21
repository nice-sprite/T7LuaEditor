#define STB_IMAGE_IMPLEMENTATION

#include "Texture.h"

bool load_texture(const char *filename,
                  ID3D11Device *device,
                  ID3D11ShaderResourceView **srv,
                  int *width,
                  int *height) {
  // Load from disk into a raw RGBA buffer
  int image_width = 0;
  int image_height = 0;
  unsigned char *image_data =
      stbi_load(filename, &image_width, &image_height, NULL, 4);
  if (image_data == NULL)
    return false;

  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = image_width;
  desc.Height = image_height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags = 0;

  ID3D11Texture2D *texture;
  D3D11_SUBRESOURCE_DATA sr;
  sr.pSysMem = image_data;
  sr.SysMemPitch = desc.Width * 4;
  sr.SysMemSlicePitch = 0;
  device->CreateTexture2D(&desc, &sr, &texture);

  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
  srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = desc.MipLevels;
  srvDesc.Texture2D.MostDetailedMip = 0;
  device->CreateShaderResourceView(texture, &srvDesc, srv);

  *width = image_width;
  *height = image_height;
  stbi_image_free(image_data);
  return true;
}
