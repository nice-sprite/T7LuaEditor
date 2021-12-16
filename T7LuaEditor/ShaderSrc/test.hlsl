cbuffer ConstantBuffer
{
    float4x4 modelViewProjection;
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
    output.uv = texcoord;
    return output;
}


Texture2D Texture;
SamplerState ss;

float4 ps_main(float4 position : SV_POSITION, float4 color : COLOR, float2 uv : TEXCOORD) : SV_TARGET
{

    return Texture.Sample(ss, uv);
}

