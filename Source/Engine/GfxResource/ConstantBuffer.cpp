//
// Created by coxtr on 11/23/2021.
//

#include "ConstantBuffer.h"


void bind_constant_buffer(ID3D11DeviceContext *context, int bufferSlot, ID3D11Buffer *buffer) 
{
    context->VSSetConstantBuffers(bufferSlot, 1, &buffer);
    context->VSSetConstantBuffers(bufferSlot, 1, &buffer);
}
