cbuffer ConstantBuffer : register(b0)
{
    float4x4 modelViewProjection;
    float4   timeTickDeltaFrame;
    float2   viewportSize;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};


PSInput vs_main(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
    PSInput output;
    position.w = 1.0;

    output.position = mul(modelViewProjection, position);
    output.color = color;
    output.uv = texcoord; // multiply the uv grid factor here, not in PS
    return output;
}

// D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT = 128
// this will probably end up being a utility atlas for icons, but for now 
Texture2D<float4> GridTexture : register(t0); 
Texture2D<float4> BatchAtlas : register(t1);

// D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT = 16

SamplerState linearWrapUV : register(s0);


float4 ps_main(float4 position : SV_POSITION, float4 color : COLOR, float2 uv : TEXCOORD) : SV_TARGET
{
    return GridTexture.Sample(linearWrapUV, uv) + color;
}

