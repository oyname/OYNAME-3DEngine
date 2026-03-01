// VertexShaderSkinning.hlsl - giDX Engine
// Identisch mit VertexShader.hlsl, jedoch mit Skelett-Skinning.
// Registers: b0 (Matrices), b1 (Lights), b2 (Material), b3 (Shadow Matrices), b4 (Bones)

// ==================== CONSTANT BUFFERS ====================

cbuffer ConstantBuffer : register(b0)
{
    row_major float4x4 _viewMatrix;
    row_major float4x4 _projectionMatrix;
    row_major float4x4 _worldMatrix;
};

struct LightData
{
    float4 lightPosition;
    float4 lightDirection;
    float4 lightDiffuseColor;
    float4 lightAmbientColor;
};

cbuffer LightBuffer : register(b1)
{
    LightData lights[32];
    uint lightCount;
    float3 lightPadding;
};

cbuffer MaterialBuffer : register(b2)
{
    float4 gBaseColor;
    float4 gSpecularColor;
    float4 gEmissiveColor;
    float4 gUvTilingOffset;
    float4 gPbr;
    float4 gAlpha;
    uint4  gTexIndex;
    uint4  gMisc;
};

cbuffer ShadowMatrixBuffer : register(b3)
{
    row_major float4x4 lightViewMatrix;
    row_major float4x4 lightProjectionMatrix;
};

// Bone-Matrizen (max 128, Register b4)
cbuffer BoneBuffer : register(b4)
{
    row_major float4x4 boneMatrices[128];
};

// ==================== INPUT / OUTPUT STRUCTURES ====================

struct VS_INPUT
{
    float3   position     : POSITION;
    float3   normal       : NORMAL;
    float4   tangent      : TANGENT;
    float4   color        : COLOR;
    float2   texCoord     : TEXCOORD0;
    float2   texCoord2    : TEXCOORD1;
    uint4    boneIndices  : BLENDINDICES;
    float4   boneWeights  : BLENDWEIGHT;
};

struct VS_OUTPUT
{
    float4 position          : SV_POSITION;
    float3 normal            : NORMAL;
    float4 tangent           : TEXCOORD5;
    float3 worldPosition     : TEXCOORD1;
    float4 color             : COLOR;
    float2 texCoord          : TEXCOORD0;
    float4 positionLightSpace: TEXCOORD2;
    float3 viewDirection     : TEXCOORD3;
    float2 texCoord2         : TEXCOORD4;
};

// ==================== SKINNING HELPER ====================

// Berechnet die geskinnte Position und Normale aus bis zu 4 Bones.
void Skin(float3 inPos, float3 inNorm,
          uint4 idx, float4 weights,
          out float3 outPos, out float3 outNorm)
{
    outPos  = float3(0, 0, 0);
    outNorm = float3(0, 0, 0);

    outPos  += mul(float4(inPos,  1.0f), boneMatrices[idx.x]).xyz * weights.x;
    outPos  += mul(float4(inPos,  1.0f), boneMatrices[idx.y]).xyz * weights.y;
    outPos  += mul(float4(inPos,  1.0f), boneMatrices[idx.z]).xyz * weights.z;
    outPos  += mul(float4(inPos,  1.0f), boneMatrices[idx.w]).xyz * weights.w;

    outNorm += mul(inNorm, (float3x3)boneMatrices[idx.x]) * weights.x;
    outNorm += mul(inNorm, (float3x3)boneMatrices[idx.y]) * weights.y;
    outNorm += mul(inNorm, (float3x3)boneMatrices[idx.z]) * weights.z;
    outNorm += mul(inNorm, (float3x3)boneMatrices[idx.w]) * weights.w;

    outNorm = normalize(outNorm);
}

// ==================== MAIN VERTEX SHADER ====================

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    // Skinning: Position und Normale in Bone-Space transformieren
    float3 skinnedPos;
    float3 skinnedNorm;
    Skin(input.position, input.normal,
         input.boneIndices, input.boneWeights,
         skinnedPos, skinnedNorm);

    // Ab hier identisch mit VertexShader.hlsl
    float4 worldPos = mul(float4(skinnedPos, 1.0f), _worldMatrix);
    o.worldPosition = worldPos.xyz;

    o.position = mul(worldPos, _viewMatrix);
    o.position = mul(o.position, _projectionMatrix);

    o.normal = normalize(mul(skinnedNorm, (float3x3)_worldMatrix));

    float3 tW = normalize(mul(input.tangent.xyz, (float3x3)_worldMatrix));
    o.tangent = float4(tW, input.tangent.w);

    o.color     = input.color;
    o.texCoord  = input.texCoord;
    o.texCoord2 = input.texCoord2;

    float4 lightViewPos  = mul(worldPos, lightViewMatrix);
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
