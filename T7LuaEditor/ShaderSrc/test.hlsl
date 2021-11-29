cbuffer ConstantBuffer
{
    float4x4 worldMatrix;
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
};

struct VSInput
{
    float4 pos : POSITION;
    float4 color: COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


VSInput vs_main(VSInput vin)
{
    PSInput output;

    vin.pos.w = 1.0f;
    output.position = mul(mul(vin.pos, worldMatrix), projectionMatrix);
    output.color = vin.color;
}


float4 ps_main(PSInput pin) : SV_TARGET
{
    return pin.color;
}

