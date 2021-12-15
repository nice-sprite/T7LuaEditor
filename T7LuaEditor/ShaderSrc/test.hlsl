cbuffer ConstantBuffer
{
    float time;
}

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


PSInput vs_main(float4 position : POSITION, float4 color : COLOR)
{
    PSInput output;

    output.position = position;
    output.position.x = output.position.x + sin(time*0.01);
    output.position.w = 1.0;
    output.color = color;
    return output;
}


float4 ps_main(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return color;
}

