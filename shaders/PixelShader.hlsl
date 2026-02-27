// PixelShader.hlsl - Lighting + Shadow Mapping (PCF)
// Registers:
//  - t0/s0 : base color (albedo)
//  - t1/s1 : detail/lightmap/extra (optional)
//  - t2/s2 : roughness map (optional)
//  - t3/s3 : metallic map  (optional)
//  - t7/s7 : shadow map (comparison sampler)
//  - b1    : LightBuffer
//  - b2    : MaterialBuffer (keep in sync with Material::MaterialData)

struct LightData
{
    float4 lightPosition; // XYZ: Position, W: 0 directional, 1 point
    float4 lightDirection; // XYZ: Direction (for directional lights)
    float4 lightDiffuseColor; // RGB: Diffuse, A: radius (for point lights)
    float4 lightAmbientColor; // RGB: Ambient (only first light used)
};

cbuffer LightBuffer : register(b1)
{
    LightData lights[32];
    uint lightCount;
    float3 lightPadding; // 16-byte alignment
};

cbuffer MaterialBuffer : register(b2)
{
    float4 gBaseColor;       // baseColor
    float4 gSpecularColor;

    float4 gEmissiveColor;
    float4 gUvTilingOffset;  // xy tiling, zw offset

    float  gMetallic;
    float  gRoughness;
    float  gNormalScale;
    float  gOcclusionStrength;

    float  gShininess;
    float  gTransparency;
    float  gAlphaCutoff;
    float  gReceiveShadows;

    float  gBlendMode;
    float  gBlendFactor;
    uint   gMaterialFlags;
    float  _pad0;
};

// Flags (must match Material::MaterialFlags)
#define MF_ALPHA_TEST      (1u<<0)
#define MF_DOUBLE_SIDED    (1u<<1)
#define MF_UNLIT           (1u<<2)
#define MF_USE_NORMAL_MAP  (1u<<3)
#define MF_USE_ORM_MAP     (1u<<4)
#define MF_USE_EMISSIVE    (1u<<5)

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 tangent : TEXCOORD5; // world tangent xyz + handedness
    float3 worldPosition : TEXCOORD1;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
    float4 positionLightSpace : TEXCOORD2;
    float3 viewDirection : TEXCOORD3;
    float2 texCoord2 : TEXCOORD4; // Lightmap / Detail UV
};

Texture2D textureMap : register(t0); // Albedo / Diffuse
SamplerState samplerState : register(s0);

// Slot 1: Normal Map (Tangent Space)  (uses UV0 / TEXCOORD0)
Texture2D textureMap2 : register(t1);
SamplerState samplerState2 : register(s1);

// Slot 2: Roughness Map  (teilt UV mit t0)
Texture2D textureMap3 : register(t2);
SamplerState samplerState3 : register(s2);

// Slot 3: Metallic Map   (teilt UV mit t0)
Texture2D textureMap4 : register(t3);
SamplerState samplerState4 : register(s3);

// Shadow map moved to t7/s7 to avoid collision with material multi-textures
Texture2D shadowMapTexture : register(t7);
SamplerComparisonState shadowSampler : register(s7);

// Point light falloff (simple quadratic)
float CalculateLightFalloff(float distance, float radius)
{
    float attenuation = max(0.0f, 1.0f - (distance / radius));
    return attenuation * attenuation;
}

// Shadow factor using PCF + comparison sampling.
// Returns 1.0 = lit, 0.0 = fully shadowed.
float CalculateShadowFactor(float4 positionLightSpace, float3 normal, float3 lightDir)
{
    // Avoid div-by-zero / invalid clip
    if (positionLightSpace.w <= 0.00001f)
        return 1.0f;

    // Perspective divide -> NDC
    float3 projCoords = positionLightSpace.xyz / positionLightSpace.w;

    // NDC [-1..1] -> texture [0..1]
    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = -projCoords.y * 0.5f + 0.5f;

    // Outside shadow map => lit
    if (projCoords.x < 0.0f || projCoords.x > 1.0f ||
        projCoords.y < 0.0f || projCoords.y > 1.0f)
        return 1.0f;

    // Behind near/far => lit
    if (projCoords.z < 0.0f || projCoords.z > 1.0f)
        return 1.0f;

    // Slope-scaled-ish bias (cheap)
    // lightDir is direction from light->surface for directional (see C++), we use surface->light as -lightDir
    float ndotl = saturate(dot(normalize(normal), normalize(-lightDir)));
    float bias = max(0.0005f, 0.0030f * (1.0f - ndotl));

    float compareDepth = projCoords.z - bias;

    // PCF 3x3
    uint w, h;
    shadowMapTexture.GetDimensions(w, h);
    float2 texelSize = 1.0f / float2((float) w, (float) h);

    float shadowSum = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float2 uv = projCoords.xy + float2((float) x, (float) y) * texelSize;
            // SampleCmp returns 1 if compareDepth <= sampledDepth (lit), else 0 (shadow)
            shadowSum += shadowMapTexture.SampleCmpLevelZero(shadowSampler, uv, compareDepth);
        }
    }

    return shadowSum / 9.0f;
}

float4 main(PS_INPUT input) : SV_Target
{
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(input.viewDirection);

    // --- Normal Mapping (Tangent Space) ---
    if ((gMaterialFlags & MF_USE_NORMAL_MAP) != 0u && gNormalScale > 0.0001f)
    {
        // Sample tangent-space normal and unpack [0..1] -> [-1..1]
        float3 nTS = textureMap2.Sample(samplerState2, input.texCoord).xyz * 2.0f - 1.0f;

        // Scale strength (XY is typical; Z stays to keep length)
        nTS.xy *= gNormalScale;
        nTS = normalize(nTS);

        float3 N = normalize(input.normal);
        float3 T = normalize(input.tangent.xyz);
        float3 B = normalize(cross(N, T) * input.tangent.w);

        // Transform to world space
        normal = normalize(nTS.x * T + nTS.y * B + nTS.z * N);
    }

    float3 ambient = float3(0.0, 0.0, 0.0);
    float3 diffuseAccum = float3(0.0, 0.0, 0.0);
    float3 specularAccum = float3(0.0, 0.0, 0.0);

    if (lightCount > 0)
        ambient = lights[0].lightAmbientColor.rgb;

    // Index des ersten Directional Light finden (fuer Shadow Mapping)
    int shadowLightIndex = -1;
    float3 shadowLightDir = float3(0, -1, 0);
    [loop]
    for (uint s = 0; s < lightCount; ++s)
    {
        if (lights[s].lightPosition.w < 0.5f) // Directional
        {
            shadowLightIndex = (int) s;
            shadowLightDir = normalize(lights[s].lightDirection.xyz);
            break;
        }
    }

    for (uint i = 0; i < lightCount; ++i)
    {
        float3 lightDir;
        float lightIntensity = 1.0f;

        // W: 0 directional, 1 point
        if (lights[i].lightPosition.w > 0.5f)
        {
            float3 lightToPixel = input.worldPosition - lights[i].lightPosition.xyz;
            float distance = length(lightToPixel);
            lightDir = normalize(lightToPixel);

            float radius = (lights[i].lightDiffuseColor.a > 0.1f) ? lights[i].lightDiffuseColor.a : 100.0f;
            lightIntensity = CalculateLightFalloff(distance, radius);
        }
        else
        {
            lightDir = normalize(lights[i].lightDirection.xyz);
        }

        // Shadow nur fuer das Directional Light das Schatten wirft
        float shadowFactor = 1.0f;
        if (gReceiveShadows > 0.5f && (int) i == shadowLightIndex)
            shadowFactor = CalculateShadowFactor(input.positionLightSpace, normal, shadowLightDir);

        // Diffuse (Lambert)
        float diffuse_factor = max(dot(normal, -lightDir), 0.0f);
        diffuseAccum += lights[i].lightDiffuseColor.rgb * diffuse_factor * lightIntensity * shadowFactor;

        // Specular
        float3 halfVec = normalize(-lightDir + viewDir);
        float specular_factor = pow(max(dot(normal, halfVec), 0.0f), max(gShininess, 1.0f));

        // Optional but recommended: gate specular by NdotL so back-facing light doesn't create highlights.
        float NdotL = diffuse_factor;

        specularAccum += gSpecularColor.rgb
                       * specular_factor
                       * lights[i].lightDiffuseColor.rgb
                       * lightIntensity
                       * shadowFactor
                       * NdotL;
    }

    float2 uv0 = input.texCoord * gUvTilingOffset.xy + gUvTilingOffset.zw;

    // Ambient occlusion (optional, cheapest possible integration)
    float ao = 1.0f;
    if ((gMaterialFlags & MF_USE_ORM_MAP) != 0u)
    {
        float4 roughSamp = textureMap3.Sample(samplerState3, uv0);
        // Convention: roughness in .r, AO in .g (if present)
        ao = lerp(1.0f, roughSamp.g, saturate(gOcclusionStrength));
    }

    float3 lighting = ((gMaterialFlags & MF_UNLIT) != 0u)
        ? float3(1.0f, 1.0f, 1.0f)
        : saturate(ambient * ao + diffuseAccum + specularAccum);
    float4 texColor = textureMap.Sample(samplerState, uv0);
    // (slot1 is now NormalMap; detail/lightmap blending removed here)

    // Optional alpha test (classic cutout)
    if ((gMaterialFlags & MF_ALPHA_TEST) != 0u)
    {
        float a = texColor.a * gBaseColor.a;
        if (a < gAlphaCutoff)
            discard;
    }

    float3 albedo = texColor.rgb * gBaseColor.rgb * input.color.rgb;
    float3 final_color = albedo * lighting;

    if ((gMaterialFlags & MF_USE_EMISSIVE) != 0u)
        final_color += gEmissiveColor.rgb;

    float finalAlpha = saturate(texColor.a * gBaseColor.a * gTransparency);
    return float4(saturate(final_color), finalAlpha);
}

