#ifndef TEXTURE_H
#define TEXTURE_H

#include <stb/stb_image.h>
#include <d3d11.h>

bool load_texture(const char *filename, ID3D11Device *device, ID3D11ShaderResourceView **srv, int *width, int *height);

#endif
