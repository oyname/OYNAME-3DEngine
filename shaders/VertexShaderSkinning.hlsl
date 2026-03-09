// VertexShaderSkinning.hlsl - giDX Engine
// Skinned-Variante des schlanken Standard-VS.
// Pflicht-Streams: POSITION, NORMAL, COLOR, TEXCOORD0, BLENDINDICES, BLENDWEIGHT
// Keine harten Anforderungen mehr an TANGENT oder TEXCOORD1.
// Registers: b0 (Matrices), b3 (Shadow Matrices), b4 (Bones)

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

cbuffer BoneBuffer : register(b4)
{
    row_major float4x4 boneMatrices[128];
};

struct VS_INPUT
{
    float3 position    : POSITION;
    float3 normal      : NORMAL;
    float4 color       : COLOR;
    float2 texCoord    : TEXCOORD0;
    uint4  boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
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

void Skin(float3 inPos, float3 inNorm,
          uint4 idx, float4 weights,
          out float3 outPos, out float3 outNorm)
{
    outPos = float3(0, 0, 0);
    outNorm = float3(0, 0, 0);

    outPos += mul(float4(inPos, 1.0f), boneMatrices[idx.x]).xyz * weights.x;
    outPos += mul(float4(inPos, 1.0f), boneMatrices[idx.y]).xyz * weights.y;
    outPos += mul(float4(inPos, 1.0f), boneMatrices[idx.z]).xyz * weights.z;
    outPos += mul(float4(inPos, 1.0f), boneMatrices[idx.w]).xyz * weights.w;

    outNorm += mul(inNorm, (float3x3)boneMatrices[idx.x]) * weights.x;
    outNorm += mul(inNorm, (float3x3)boneMatrices[idx.y]) * weights.y;
    outNorm += mul(inNorm, (float3x3)boneMatrices[idx.z]) * weights.z;
    outNorm += mul(inNorm, (float3x3)boneMatrices[idx.w]) * weights.w;

    outNorm = normalize(outNorm);
}

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    float3 skinnedPos;
    float3 skinnedNorm;
    Skin(input.position, input.normal,
         input.boneIndices, input.boneWeights,
         skinnedPos, skinnedNorm);

    float4 worldPos = mul(float4(skinnedPos, 1.0f), _worldMatrix);
    o.worldPosition = worldPos.xyz;

    o.position = mul(worldPos, _viewMatrix);
    o.position = mul(o.position, _projectionMatrix);

    o.normal = normalize(mul(skinnedNorm, (float3x3)_worldMatrix));
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
