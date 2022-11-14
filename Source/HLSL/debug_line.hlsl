cbuffer ConstantBuffer : register(b0) {
  float4x4 modelViewProjection;
};

struct PSInput {
  float4 position : SV_POSITION;
  float4 color: COLOR; 
};

PSInput vs_main(float4 position: POSITION, float4 color: COLOR) {
  PSInput output;

  position.w = 1.0;
  output.position = mul(modelViewProjection, position);
  output.color = color;
  return output; 
}

float4 ps_main(float4 position : SV_POSITION, float4 color: COLOR) : SV_TARGET {
  return color;
}