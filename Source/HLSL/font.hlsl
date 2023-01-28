cbuffer ConstantBuffer : register(b0) {
  float4x4 modelViewProjection;
};

struct PSInput {

  float4 position : SV_POSITION;
  float4 color : COLOR;
  float2 uv : TEXCOORD;
}; 

// D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT = 128
// this will probably end up being a utility atlas for icons, but for now 
Texture2D<float4> Font_Texture: register(t0); 
Texture2D<float4> BatchAtlas : register(t1);
SamplerState Sampler;

PSInput vs_main(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD)
{
  PSInput output; 
  position.w = 1.0;

  output.position = mul(modelViewProjection, position);
  output.color = color;
  output.uv = texcoord; // multiply the uv grid factor here, not in PS
  return output;
}

float4 ps_main(PSInput input) : SV_TARGET {
  float4 color_intensity = Font_Texture.Sample(Sampler, input.uv).rrrr;
  return smoothstep(0.45, 0.5, color_intensity) * input.color;
}
