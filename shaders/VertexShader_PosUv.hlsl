// VertexShader_PosUv.hlsl
cbuffer MatrixBuffer : register(b0)
{
    row_major float4x4 viewMatrix;
    row_major float4x4 projectionMatrix;
    row_major float4x4 worldMatrix;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    float4 p = float4(input.position, 1.0f);
    p = mul(p, worldMatrix);
    p = mul(p, viewMatrix);
    p = mul(p, projectionMatrix);

    o.position = p;
    o.texCoord = input.texCoord;

    return o;
}