// PixelShader.hlsl - Lighting + Shadow Mapping (PCF)
// Standard-PS passend zum schlanken VS-Vertrag:
//  - nur UV0 ist garantiert
//  - zweite Textur nutzt UV0 als Fallback
//  - Normal-Mapping rekonstruiert TBN aus ddx/ddy statt Tangent-Vertexstream
// Registers:
//  - t0 / s0 : Albedo texture + sampler
//  - t1      : Zweite Textur (Decal/Blend)
//  - t2      : Normal map
//  - t3      : ORM map
//  - t4      : Occlusion map
//  - t5      : Roughness map
//  - t6      : Metallic map
//  - t16/s7  : Shadow map + comparison sampler
//  - b1      : LightBuffer
//  - b2      : MaterialBuffer

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
    float3 _padLight;
};

cbuffer MaterialBuffer : register(b2)
{
    float4 gBaseColor;
    float4 gSpecularColor;
    float4 gEmissiveColor;
    float4 gUvTilingOffset;

    float4 gPbr;
    float4 gAlpha;

    uint4 gTexIndex;
    uint4 gMisc;
};

#define MF_ALPHA_TEST        (1u<<0)
#define MF_DOUBLE_SIDED      (1u<<1)
#define MF_UNLIT             (1u<<2)
#define MF_USE_NORMAL_MAP    (1u<<3)
#define MF_USE_ORM_MAP       (1u<<4)
#define MF_USE_EMISSIVE      (1u<<5)
#define MF_USE_OCCLUSION_MAP (1u<<7)
#define MF_USE_ROUGHNESS_MAP (1u<<8)
#define MF_USE_METALLIC_MAP  (1u<<9)
#define MF_SHADING_PBR       (1u<<10)
#define ROUGHNESS_MIN 0.04f

struct PS_INPUT
{
    float4 position           : SV_POSITION;
    float3 normal             : NORMAL;
    float3 worldPosition      : TEXCOORD1;
    float4 color              : COLOR;
    float2 texCoord           : TEXCOORD0;
    float4 positionLightSpace : TEXCOORD2;
    float3 viewDirection      : TEXCOORD3;
};

Texture2D gAlbedo     : register(t0);
Texture2D gDecal      : register(t1);
Texture2D gNormalMap  : register(t2);
Texture2D gORM        : register(t3);
Texture2D gOcclusion  : register(t4);
Texture2D gRoughness  : register(t5);
Texture2D gMetallic   : register(t6);
SamplerState gSampler : register(s0);

Texture2D shadowMapTexture : register(t16);
SamplerComparisonState shadowSampler : register(s7);

static const float PI = 3.14159265359f;

float CalculateLightFalloff(float distance, float radius)
{
    float attenuation = max(0.0f, 1.0f - (distance / radius));
    return attenuation * attenuation;
}

float CalculateShadowFactor(float4 positionLightSpace, float3 normal, float3 lightDir)
{
    if (positionLightSpace.w <= 0.00001f)
        return 1.0f;

    float3 projCoords = positionLightSpace.xyz / positionLightSpace.w;
    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = -projCoords.y * 0.5f + 0.5f;

    if (projCoords.x < 0.0f || projCoords.x > 1.0f ||
        projCoords.y < 0.0f || projCoords.y > 1.0f ||
        projCoords.z < 0.0f || projCoords.z > 1.0f)
        return 1.0f;

    float ndotl = saturate(dot(normalize(normal), normalize(-lightDir)));
    float bias = max(0.0005f, 0.0030f * (1.0f - ndotl));
    float compareDepth = projCoords.z - bias;

    uint w, h;
    shadowMapTexture.GetDimensions(w, h);
    float2 texelSize = 1.0f / float2((float)w, (float)h);

    float shadowSum = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            float2 uv = projCoords.xy + float2((float)x, (float)y) * texelSize;
            shadowSum += shadowMapTexture.SampleCmpLevelZero(shadowSampler, uv, compareDepth);
        }
    }
    return shadowSum / 9.0f;
}

float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0f) + 1.0f;
    return a2 / max(PI * denom * denom, 1e-6f);
}

float GeometrySchlickGGX(float NdotX, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return NdotX / max(NdotX * (1.0f - k) + k, 1e-6f);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggxV = GeometrySchlickGGX(NdotV, roughness);
    float ggxL = GeometrySchlickGGX(NdotL, roughness);
    return ggxV * ggxL;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float3x3 BuildCotangentFrame(float3 N, float3 worldPos, float2 uv)
{
    float3 dp1 = ddx(worldPos);
    float3 dp2 = ddy(worldPos);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);

    float det = duv1.x * duv2.y - duv1.y * duv2.x;
    if (abs(det) < 1e-8f)
    {
        float3 up = (abs(N.y) < 0.999f) ? float3(0, 1, 0) : float3(1, 0, 0);
        float3 T = normalize(cross(up, N));
        float3 B = normalize(cross(N, T));
        return float3x3(T, B, N);
    }

    float invDet = 1.0f / det;
    float3 T = normalize((dp1 * duv2.y - dp2 * duv1.y) * invDet);
    float3 B = normalize((-dp1 * duv2.x + dp2 * duv1.x) * invDet);

    T = normalize(T - N * dot(N, T));
    B = normalize(B - N * dot(N, B));
    return float3x3(T, B, N);
}

float4 main(PS_INPUT input) : SV_Target
{
    const uint flags = gMisc.y;
    const uint blendMode = gMisc.x;
    const bool usePBR = ((flags & MF_SHADING_PBR) != 0u);
    const float blendFactor = asfloat(gMisc.z);

    float normalScale = gPbr.z;
    float occlusionStrength = gPbr.w;
    float shininess = gAlpha.x;
    float transparency = gAlpha.y;
    float alphaCutoff = gAlpha.z;
    float receiveShadow = gAlpha.w;

    float metallic = saturate(gPbr.x);
    float roughness = max(saturate(gPbr.y), ROUGHNESS_MIN);
    float2 uv0 = input.texCoord * gUvTilingOffset.xy + gUvTilingOffset.zw;
    float2 uv1 = uv0;

    float3 N = normalize(input.normal);

    if ((flags & MF_USE_NORMAL_MAP) != 0u)
    {
        float3x3 tbn = BuildCotangentFrame(N, input.worldPosition, uv0);
        float3 nTS = gNormalMap.Sample(gSampler, uv0).xyz * 2.0f - 1.0f;
        nTS.xy *= normalScale;
        float3 T = tbn[0];
        float3 B = tbn[1];
        float3 NN = tbn[2];
        N = normalize(nTS.x * T + nTS.y * B + nTS.z * NN);
    }

    float3 viewDir = normalize(input.viewDirection);
    float4 texColor = gAlbedo.Sample(gSampler, uv0);

    if ((flags & MF_ALPHA_TEST) != 0u)
    {
        float a = texColor.a * gBaseColor.a;
        if (a < alphaCutoff)
            discard;
    }

    if (blendMode > 0u)
    {
        float4 tex2 = gDecal.Sample(gSampler, uv1);
        float f = saturate(blendFactor);

        if (blendMode == 1u)
            texColor.rgb = lerp(texColor.rgb, texColor.rgb * tex2.rgb, f);
        else if (blendMode == 2u)
            texColor.rgb = lerp(texColor.rgb, texColor.rgb * tex2.rgb * 2.0f, f);
        else if (blendMode == 3u)
            texColor.rgb = lerp(texColor.rgb, texColor.rgb + tex2.rgb, f);
        else if (blendMode == 4u)
            texColor.rgb = lerp(texColor.rgb, tex2.rgb, tex2.a * f);
        else if (blendMode == 5u)
        {
            float lum = dot(tex2.rgb, float3(0.2126f, 0.7152f, 0.0722f));
            texColor.rgb = lerp(texColor.rgb, texColor.rgb + tex2.rgb * lum, f);
        }
    }

    float3 albedo = texColor.rgb * gBaseColor.rgb * input.color.rgb;

    float3 ambient = float3(0, 0, 0);
    float3 diffuseAccum = float3(0, 0, 0);
    float3 specularAccum = float3(0, 0, 0);

    if (lightCount > 0)
        ambient = lights[0].lightAmbientColor.rgb;

    int shadowLightIndex = -1;
    float3 shadowLightDir = float3(0, -1, 0);
    [loop]
    for (uint s = 0; s < lightCount; ++s)
    {
        if (lights[s].lightPosition.w < 0.5f)
        {
            shadowLightIndex = (int)s;
            shadowLightDir = normalize(lights[s].lightDirection.xyz);
            break;
        }
    }

    float ao = 1.0f;
    if ((flags & MF_USE_ORM_MAP) != 0u)
    {
        float3 orm = gORM.Sample(gSampler, uv0).rgb;
        ao = lerp(1.0f, orm.r, saturate(occlusionStrength));
        roughness = max(orm.g, ROUGHNESS_MIN);
        metallic = saturate(orm.b);
    }
    else
    {
        if ((flags & MF_USE_OCCLUSION_MAP) != 0u)
        {
            float occ = gOcclusion.Sample(gSampler, uv0).r;
            ao = lerp(1.0f, occ, saturate(occlusionStrength));
        }
        if ((flags & MF_USE_ROUGHNESS_MAP) != 0u)
            roughness = max(gRoughness.Sample(gSampler, uv0).r, ROUGHNESS_MIN);
        if ((flags & MF_USE_METALLIC_MAP) != 0u)
            metallic = saturate(gMetallic.Sample(gSampler, uv0).r);
    }

    for (uint i = 0; i < lightCount; ++i)
    {
        float3 lightDir;
        float lightIntensity = 1.0f;

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

        float shadowFactor = 1.0f;
        if (receiveShadow > 0.5f && (int)i == shadowLightIndex)
            shadowFactor = CalculateShadowFactor(input.positionLightSpace, N, shadowLightDir);

        float NdotL = max(dot(N, -lightDir), 0.0f);

        if (!usePBR)
        {
            diffuseAccum += lights[i].lightDiffuseColor.rgb * NdotL * lightIntensity * shadowFactor;
            float3 halfVec = normalize(-lightDir + viewDir);
            float spec = pow(max(dot(N, halfVec), 0.0f), max(shininess, 1.0f));
            specularAccum += gSpecularColor.rgb * spec * lights[i].lightDiffuseColor.rgb * lightIntensity * shadowFactor * NdotL;
        }
        else
        {
            float3 V = viewDir;
            float3 L = normalize(-lightDir);
            float3 H = normalize(V + L);

            float NdotV = max(dot(N, V), 0.0f);
            float NdotH = max(dot(N, H), 0.0f);
            float VdotH = max(dot(V, H), 0.0f);

            float3 dielectricF0 = float3(0.04f, 0.04f, 0.04f);
            float3 legacyF0 = clamp(saturate(gSpecularColor.rgb), 0.02f, 0.08f);
            float3 F0 = lerp(dielectricF0, albedo, metallic);
            F0 = lerp(F0, legacyF0, 0.20f);

            float D = DistributionGGX(NdotH, roughness);
            float G = GeometrySmith(NdotV, NdotL, roughness);
            float3 F = FresnelSchlick(VdotH, F0);

            float3 numerator = D * G * F;
            float denom = max(4.0f * NdotV * NdotL, 1e-6f);
            float3 specBRDF = numerator / denom;

            diffuseAccum += lights[i].lightDiffuseColor.rgb * (NdotL * lightIntensity * shadowFactor);
            specularAccum += specBRDF * lights[i].lightDiffuseColor.rgb * (NdotL * lightIntensity * shadowFactor);
        }
    }

    float3 lighting = ((flags & MF_UNLIT) != 0u)
        ? float3(1, 1, 1)
        : (usePBR ? float3(1, 1, 1) : saturate(ambient * ao + diffuseAccum + specularAccum));

    float3 final_color;
    if (!usePBR)
    {
        final_color = albedo * lighting;
    }
    else
    {
        float3 dielectricF0 = float3(0.04f, 0.04f, 0.04f);
        float3 legacyF0 = clamp(saturate(gSpecularColor.rgb), 0.02f, 0.08f);
        float3 F0 = lerp(dielectricF0, albedo, metallic);
        F0 = lerp(F0, legacyF0, 0.20f);

        float3 kS = saturate(F0);
        float3 kD = (1.0f - kS) * (1.0f - metallic);
        float3 diffuseTerm = (kD * albedo / PI) * diffuseAccum;
        float3 specularTerm = specularAccum;
        float3 ambientDiffuse = kD * albedo * ambient * ao;
        float3 F_ambient = FresnelSchlick(max(dot(N, viewDir), 0.0f), F0);
        float specAmbientScale = (1.0f - roughness * roughness);
        float3 ambientSpecular = F_ambient * ambient * specAmbientScale * ao;
        float3 ambientTerm = ambientDiffuse + ambientSpecular;
        final_color = ambientTerm + diffuseTerm + specularTerm;
    }

    if ((flags & MF_USE_EMISSIVE) != 0u)
        final_color += gEmissiveColor.rgb;

    float finalAlpha = saturate(texColor.a * gBaseColor.a * transparency);
    return float4(saturate(final_color), finalAlpha);
}
