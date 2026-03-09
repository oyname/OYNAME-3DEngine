// VertexShader.hlsl - giDX Engine
// Schlanker Standard-VS passend zur aktuellen Engine-Praxis:
// Pflicht-Streams: POSITION, NORMAL, COLOR, TEXCOORD0
// Optionale Daten wie Tangent / TEXCOORD1 werden nicht mehr vorausgesetzt.
// Normal-Mapping nutzt im Pixel-Shader eine aus Ableitungen rekonstruierte TBN-Basis.
// Registers: b0 (Matrices), b3 (Shadow Matrices)

cbuffer ConstantBuffer : register(b0)
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
    float3 normal   : NORMAL;
    float4 color    : COLOR;
    float2 texCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position           : SV_POSITION;
    float3 normal             : NORMAL;
    float3 worldPosition      : TEXCOORD1;
    float4 color              : COLOR;
    float2 texCoord           : TEXCOORD0;
    float4 positionLightSpace : TEXCOORD2;
    float3 viewDirection      : TEXCOORD3;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    float4 worldPos = mul(float4(input.position, 1.0f), _worldMatrix);
    o.worldPosition = worldPos.xyz;

    o.position = mul(worldPos, _viewMatrix);
    o.position = mul(o.position, _projectionMatrix);

    o.normal = normalize(mul(input.normal, (float3x3)_worldMatrix));
    o.color = input.color;
    o.texCoord = input.texCoord;

    float4 lightViewPos = mul(worldPos, lightViewMatrix);
    o.positionLightSpace = mul(lightViewPos, lightProjectionMatrix);

    float3 vt = float3(_viewMatrix[3][0], _viewMatrix[3][1], _viewMatrix[3][2]);
    float3 cameraPosition = float3(
        -dot(float3(_viewMatrix[0][0], _viewMatrix[0][1], _viewMatrix[0][2]), vt),
        -dot(float3(_viewMatrix[1][0], _viewMatrix[1][1], _viewMatrix[1][2]), vt),
        -dot(float3(_viewMatrix[2][0], _viewMatrix[2][1], _viewMatrix[2][2]), vt)
    );

    o.viewDirection = normalize(cameraPosition - worldPos.xyz);
    return o;
}
