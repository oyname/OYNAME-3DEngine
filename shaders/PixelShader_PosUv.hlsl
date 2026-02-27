// PixelShader_PosUv.hlsl
Texture2D diffuseTex : register(t0);
SamplerState sampLinear : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return diffuseTex.Sample(sampLinear, input.texCoord);
}