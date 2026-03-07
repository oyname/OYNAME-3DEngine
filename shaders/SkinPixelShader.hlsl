// SkinPixelShader.hlsl
//
// Einfacher Pixel Shader fuer Bone-Animation-Showcase.
// Nimmt die bereits im Vertex Shader berechnete Beleuchtungsfarbe
// und gibt sie direkt aus. Kein PBR, keine Texturen -- einfach und robust.

struct PSInput
{
    float4 position  : SV_POSITION;
    float4 color     : COLOR;
    float2 texcoord  : TEXCOORD0;
    float3 normal    : TEXCOORD1;
    float3 worldPos  : TEXCOORD2;
};

float4 main(PSInput input) : SV_TARGET
{
    return saturate(input.color);
}
