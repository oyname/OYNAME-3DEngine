cbuffer MatrixBuffer : register(b0)
{
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
    row_major matrix worldMatrix;
};

struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 worldPos : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    float4 p = float4(input.position, 1.0f);
    p = mul(p, worldMatrix);

    o.worldPos = p.xyz; // Weltposition fuer den Pixel Shader merken

    p = mul(p, viewMatrix);
    p = mul(p, projectionMatrix);
    o.position = p;

    return o;
}



