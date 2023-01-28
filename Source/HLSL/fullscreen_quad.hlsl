cbuffer ConstantBuffer : register(b0)
{
    float4x4 modelViewProjection;
    float4   timeTickDeltaFrame;
    float4   viewportSize;
}

float4 vs_main(uint vertex_index : SV_VERTEXID) : SV_POSITION {
  float2 texcoord = float2(vertex_index % 2, ( vertex_index %4 ) >> 1);
  float4 pos = float4( (texcoord.x - 0.5f)*2, -(texcoord.y-0.5)*2.0, 0, 1);
  return float4( pos.x , pos.y, 0, 1);
}


float4 ps_main(float4 pos: SV_POSITION) : SV_TARGET {
  
  pos.x = pos.x / viewportSize.x;
  pos.y = pos.y / viewportSize.y;

  //return float4(1.0-lerp(0.0, 0.1, pos.y), 1, 1, 1);
  return float4(0, 0.0, lerp(0.0, 0.1, pos.y), 1.0);
}
