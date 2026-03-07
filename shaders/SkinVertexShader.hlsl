// SkinVertexShader.hlsl
//
// Skinning Vertex Shader fuer Bone Animation.
//
// WICHTIG: Die Engine laedt alle Matrizen ohne XMMatrixTranspose (row-major memcpy).
// Daher muessen alle cbuffer-Matrizen als "row_major" deklariert werden,
// sonst liest HLSL die Elemente in falscher Reihenfolge.
//
// Multiplikationsreihenfolge (row-vector Konvention):
//   outPos = inPos * boneMatrix * worldMatrix * viewMatrix * projMatrix
//
// Register:
//   b0 = MatrixBuffer  (view, projection, world)
//   b1 = LightBuffer   (position, direction, diffuse, ambient)
//   b4 = BoneBuffer    (128 Bone-Matrizen)

cbuffer MatrixBuffer : register(b0)
{
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
    row_major matrix worldMatrix;
};

cbuffer LightBuffer : register(b1)
{
    float4 lightPosition;
    float4 lightDirection;
    float4 lightDiffuseColor;
    float4 lightAmbientColor;
};

cbuffer BoneBuffer : register(b4)
{
    row_major matrix boneMatrices[128];
};

struct VSInput
{
    float3 position    : POSITION;
    float3 normal      : NORMAL;
    float4 color       : COLOR;
    float2 texcoord    : TEXCOORD0;
    uint4  boneIndices : BLENDINDICES;
    float4 boneWeights : BLENDWEIGHT;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
    float2 texcoord : TEXCOORD0;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    // ---- Linear Blend Skinning ------------------------------------------
    float4 pos = float4(input.position, 1.0f);
    float3 nor = input.normal;

    float4 skinPos = float4(0, 0, 0, 0);
    float3 skinNor = float3(0, 0, 0);

    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        float w = input.boneWeights[i];
        if (w > 0.0001f)
        {
            skinPos += w * mul(pos, boneMatrices[input.boneIndices[i]]);
            skinNor += w * mul(nor, (float3x3)boneMatrices[input.boneIndices[i]]);
        }
    }
    skinPos.w = 1.0f;

    // ---- Welt -> View -> Projektion ------------------------------------
    float4 worldPos = mul(skinPos,   worldMatrix);
    float4 viewPos  = mul(worldPos,  viewMatrix);
    output.position = mul(viewPos,   projectionMatrix);

    // ---- Beleuchtung (Lambertian) --------------------------------------
    float3 worldNor = normalize(mul(skinNor, (float3x3)worldMatrix));
    float3 lightDir = normalize(-lightDirection.xyz);
    float  nDotL    = saturate(dot(worldNor, lightDir));

    float4 diffuse  = lightDiffuseColor * nDotL;
    float4 ambient  = lightAmbientColor;

    output.color    = saturate(input.color * (ambient + diffuse));
    output.texcoord = input.texcoord;

    return output;
}
