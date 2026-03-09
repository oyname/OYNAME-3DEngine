// VertexShader_Shadow.hlsl - giDX Engine
// Depth-only Shadow-VS.
// Passt jetzt zur Engine-Realitaet: fuer den Shadow-Pass wird nur POSITION benoetigt.
// b0 liefert World, b3 liefert lightView/lightProjection.

cbuffer MatrixBuffer : register(b0)
{
    row_major float4x4 _viewMatrix;
    row_major float4x4 _projectionMatrix;
    row_major float4x4 _worldMatrix;
};

cbuffer ShadowMatrixBuffer : register(b3)
{
    row_major float4x4 lightViewMatrix;
    row_major float4x4 lightProjectionMatrix;
};

struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;
    float4 worldPos = mul(float4(input.position, 1.0f), _worldMatrix);
    float4 lightViewPos = mul(worldPos, lightViewMatrix);
    o.position = mul(lightViewPos, lightProjectionMatrix);
    return o;
}
