// PixelShader.hlsl - Lighting + Shadow Mapping (PCF)
// Registers:
//  - t0 / s0 : Albedo texture + sampler
//  - t1       : Normal map
//  - t2       : ORM map (Occlusion/Roughness/Metallic)
//  - t16 / s7 : Shadow map + comparison sampler
//  - b1        : LightBuffer
//  - b2        : MaterialBuffer

struct LightData
{
    float4 lightPosition; // XYZ: Position, W: 0 directional, 1 point
    float4 lightDirection; // XYZ: Direction
    float4 lightDiffuseColor; // RGB: Diffuse, A: radius (point)
    float4 lightAmbientColor; // RGB: Ambient
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
    float4 gUvTilingOffset; // xy tiling, zw offset

    float4 gPbr; // x=metallic y=roughness z=normalScale w=occlusionStrength
    float4 gAlpha; // x=shininess y=transparency z=alphaCutoff w=receiveShadows

    uint4 gTexIndex; // x=albedo y=normal z=orm w=decal (Indizes fuer C++, im Shader nicht genutzt)
    uint4 gMisc; // x=blendMode y=materialFlags z,w unused
};

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
    float2 texCoord2 : TEXCOORD4;
};

// Feste Slots – SM5.0-kompatibel (kein dynamisches Array-Indexing)
Texture2D gAlbedo : register(t0);
Texture2D gNormalMap : register(t1);
Texture2D gORM : register(t2);
SamplerState gSampler : register(s0);

Texture2D shadowMapTexture : register(t16);
SamplerComparisonState shadowSampler : register(s7);

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
        projCoords.y < 0.0f || projCoords.y > 1.0f)
        return 1.0f;

    if (projCoords.z < 0.0f || projCoords.z > 1.0f)
        return 1.0f;

    float ndotl = saturate(dot(normalize(normal), normalize(-lightDir)));
    float bias = max(0.0005f, 0.0030f * (1.0f - ndotl));
    float compareDepth = projCoords.z - bias;

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
            shadowSum += shadowMapTexture.SampleCmpLevelZero(shadowSampler, uv, compareDepth);
        }
    }
    return shadowSum / 9.0f;
}

float4 main(PS_INPUT input) : SV_Target
{
    const uint flags = gMisc.y;

    float normalScale = gPbr.z;
    float occlusionStrength = gPbr.w;
    float shininess = gAlpha.x;
    float transparency = gAlpha.y;
    float alphaCutoff = gAlpha.z;
    float receiveShadow = gAlpha.w;

    float2 uv0 = input.texCoord * gUvTilingOffset.xy + gUvTilingOffset.zw;

    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(input.viewDirection);

    //
    // Normal map (t1)
    float3 N = normalize(input.normal);
    float3 T = input.tangent.xyz;

    // Re-orthonormalize T to N
    T = normalize(T - N * dot(N, T));

    // If tangent is degenerate, fallback
    if (dot(T, T) < 1e-6f)
    {
    // pick any orthogonal basis
        float3 up = (abs(N.y) < 0.999f) ? float3(0, 1, 0) : float3(1, 0, 0);
        T = normalize(cross(up, N));
    }

    float3 B = normalize(cross(N, T)) * input.tangent.w;

    float3 nTS = gNormalMap.Sample(gSampler, uv0).xyz * 2.0f - 1.0f;
    nTS.xy *= normalScale;
    nTS = normalize(nTS);

    normal = normalize(nTS.x * T + nTS.y * B + nTS.z * N);

    //
    
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
            shadowLightIndex = (int) s;
            shadowLightDir = normalize(lights[s].lightDirection.xyz);
            break;
        }
    }

    // ORM / AO (t2)
    float ao = 1.0f;
    if ((flags & MF_USE_ORM_MAP) != 0u)
    {
        float3 orm = gORM.Sample(gSampler, uv0).rgb;
        ao = lerp(1.0f, orm.g, saturate(occlusionStrength));
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
            float radius = (lights[i].lightDiffuseColor.a > 0.1f)
                                  ? lights[i].lightDiffuseColor.a : 100.0f;
            lightIntensity = CalculateLightFalloff(distance, radius);
        }
        else
        {
            lightDir = normalize(lights[i].lightDirection.xyz);
        }

        float shadowFactor = 1.0f;
        if (receiveShadow > 0.5f && (int) i == shadowLightIndex)
            shadowFactor = CalculateShadowFactor(input.positionLightSpace, normal, shadowLightDir);

        float NdotL = max(dot(normal, -lightDir), 0.0f);
        diffuseAccum += lights[i].lightDiffuseColor.rgb * NdotL * lightIntensity * shadowFactor;

        float3 halfVec = normalize(-lightDir + viewDir);
        float spec = pow(max(dot(normal, halfVec), 0.0f), max(shininess, 1.0f));
        specularAccum += gSpecularColor.rgb * spec
                       * lights[i].lightDiffuseColor.rgb
                       * lightIntensity * shadowFactor * NdotL;
    }

    float3 lighting = ((flags & MF_UNLIT) != 0u)
        ? float3(1, 1, 1)
        : saturate(ambient * ao + diffuseAccum + specularAccum);

    // Albedo (t0)
    float4 texColor = gAlbedo.Sample(gSampler, uv0);

    if ((flags & MF_ALPHA_TEST) != 0u)
    {
        float a = texColor.a * gBaseColor.a;
        if (a < alphaCutoff)
            discard;
    }

    float3 albedo = texColor.rgb * gBaseColor.rgb * input.color.rgb;
    float3 final_color = albedo * lighting;

    if ((flags & MF_USE_EMISSIVE) != 0u)
        final_color += gEmissiveColor.rgb;

    float finalAlpha = saturate(texColor.a * gBaseColor.a * transparency);
    return float4(saturate(final_color), finalAlpha);
}
