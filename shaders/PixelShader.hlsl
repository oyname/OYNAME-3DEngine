// PixelShader.hlsl - Lighting + Shadow Mapping (PCF)
// Registers:
//  - t0 / s0 : Albedo texture + sampler
//  - t1       : Normal map  (nur wenn MF_USE_NORMAL_MAP gesetzt)
//  - t2       : ORM map     (nur wenn MF_USE_ORM_MAP gesetzt)
//  - t3       : Zweite Textur fuer Blend-Modi (blendMode > 0)
//  - t4       : Occlusion (separat) (nur wenn MF_USE_OCCLUSION_MAP gesetzt UND kein ORM)
//  - t5       : Roughness (separat) (nur wenn MF_USE_ROUGHNESS_MAP gesetzt UND kein ORM)
//  - t6       : Metallic  (separat) (nur wenn MF_USE_METALLIC_MAP  gesetzt UND kein ORM)
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

    uint4 gTexIndex; // x=albedo y=normal z=orm w=decal
    uint4 gMisc; // x=blendMode y=materialFlags z=blendFactor(bits) w=unused
};

#define MF_ALPHA_TEST      (1u<<0)
#define MF_DOUBLE_SIDED    (1u<<1)
#define MF_UNLIT           (1u<<2)
#define MF_USE_NORMAL_MAP  (1u<<3)
#define MF_USE_ORM_MAP     (1u<<4)
#define MF_USE_EMISSIVE    (1u<<5)

// Separate PBR maps (Engine Schritt 2/3)
#define MF_USE_OCCLUSION_MAP (1u<<7)
#define MF_USE_ROUGHNESS_MAP (1u<<8)
#define MF_USE_METALLIC_MAP  (1u<<9)
#define MF_SHADING_PBR      (1u<<10)
#define ROUGHNESS_MIN 0.04f

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 tangent : TEXCOORD5;
    float3 worldPosition : TEXCOORD1;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
    float4 positionLightSpace : TEXCOORD2;
    float3 viewDirection : TEXCOORD3;
    float2 texCoord2 : TEXCOORD4;
};

Texture2D gAlbedo : register(t0); // Albedo / Diffuse
Texture2D gNormalMap : register(t1); // Normal Map   (nur wenn MF_USE_NORMAL_MAP)
Texture2D gORM : register(t2); // ORM          (nur wenn MF_USE_ORM_MAP)
Texture2D gDecal : register(t3); // Zweite Textur fuer Blend-Modi
Texture2D gOcclusion : register(t4); // Separate Occlusion (R)
Texture2D gRoughness : register(t5); // Separate Roughness (R)
Texture2D gMetallic : register(t6); // Separate Metallic  (R)
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


// ==================== PBR Helpers (GGX / Smith-Schlick / Fresnel-Schlick) ====================
static const float PI = 3.14159265359f;

float DistributionGGX(float NdotH, float roughness)
{
    // Trowbridge-Reitz GGX NDF
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0f) + 1.0f;
    return a2 / max(PI * denom * denom, 1e-6f);
}

float GeometrySchlickGGX(float NdotX, float roughness)
{
    // Smith-Schlick (k from roughness)
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
    // Schlick approximation
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
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

    // PBR-Scalars (werden in dieser Shader-Version noch nicht fuer ein vollwertiges PBR genutzt,
    // aber wir modulieren sie optional durch separate Maps, damit die Daten durchgaengig sind.)
    float metallic = saturate(gPbr.x);
    float roughness = max(saturate(gPbr.y), ROUGHNESS_MIN); // GGX bricht bei exakt 0 zusammen
    float2 uv0 = input.texCoord * gUvTilingOffset.xy + gUvTilingOffset.zw;
    float2 uv1 = input.texCoord2 * gUvTilingOffset.xy + gUvTilingOffset.zw;

    // ── Normale ──────────────────────────────────────────────────────────────
    float3 N = normalize(input.normal);

    if ((flags & MF_USE_NORMAL_MAP) != 0u)
    {
        // Tangent-Fallback VOR normalize pruefen, damit NaN-Propagation verhindert wird
        float3 T = input.tangent.xyz;
        if (dot(T, T) < 1e-6f)
        {
            // Degenerierter Tangent: aus Normaler rekonstruieren
            float3 up = (abs(N.y) < 0.999f) ? float3(0, 1, 0) : float3(1, 0, 0);
            T = normalize(cross(up, N));
        }
        else
        {
            T = normalize(T);
            T = normalize(T - N * dot(N, T)); // Gram-Schmidt Re-Orthogonalisierung
        }
        float3 B = normalize(cross(N, T)) * input.tangent.w;
        float3 nTS = gNormalMap.Sample(gSampler, uv0).xyz * 2.0f - 1.0f;
        nTS.xy *= normalScale;
        N = normalize(nTS.x * T + nTS.y * B + nTS.z * N);
    }

    float3 viewDir = normalize(input.viewDirection);

    // ── Licht & Schatten ─────────────────────────────────────────────────────
    // ── Albedo (t0) ───────────────────────────────────────────────────────────
    float4 texColor = gAlbedo.Sample(gSampler, uv0);

    if ((flags & MF_ALPHA_TEST) != 0u)
    {
        float a = texColor.a * gBaseColor.a;
        if (a < alphaCutoff)
            discard;
    }

    // ── Zweite Textur / Blend-Modus (t3) ─────────────────────────────────────
    // blendMode == 0  → keine zweite Textur, texColor unveraendert
    // blendFactor     → Mischstaerke (0.0 = kein Einfluss, 1.0 = voller Einfluss)
    if (blendMode > 0u)
    {
        float4 tex2 = gDecal.Sample(gSampler, uv1);
        float f = saturate(blendFactor);

        if (blendMode == 1u)
        {
            // Multiply: Abdunkeln (Lightmap, Schatten)
            texColor.rgb = lerp(texColor.rgb, texColor.rgb * tex2.rgb, f);
        }
        else if (blendMode == 2u)
        {
            // Multiply x2: Detail-Map, Helligkeitskorrektur
            texColor.rgb = lerp(texColor.rgb, texColor.rgb * tex2.rgb * 2.0f, f);
        }
        else if (blendMode == 3u)
        {
            // Additive: Gluehen, Feuer, Licht
            texColor.rgb = lerp(texColor.rgb, texColor.rgb + tex2.rgb, f);
        }
        else if (blendMode == 4u)
        {
            // Lerp (Alpha): Decals, Aufkleber
            texColor.rgb = lerp(texColor.rgb, tex2.rgb, tex2.a * f);
        }
        else if (blendMode == 5u)
        {
            // Luminanz: Overlay mit schwarzem Hintergrund
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
            shadowLightIndex = (int) s;
            shadowLightDir = normalize(lights[s].lightDirection.xyz);
            break;
        }
    }

        // ORM / Separate PBR Inputs (Prioritaet: ORM > separate Maps > Konstanten)
    // metallic/roughness sind bereits mit Skalaren aus gPbr initialisiert.
    // Texturen modulieren diese Werte (oder ersetzen sie vollstaendig).
    float ao = 1.0f;

    if ((flags & MF_USE_ORM_MAP) != 0u)
    {
        // Standard ORM Layout: R=Occlusion, G=Roughness, B=Metallic
        float3 orm = gORM.Sample(gSampler, uv0).rgb;
        ao = lerp(1.0f, orm.r, saturate(occlusionStrength));
        roughness = max(orm.g, ROUGHNESS_MIN);
        metallic = saturate(orm.b);
    }
    else
    {
        // Separate Maps (nur aktiv wenn kein ORM gesetzt)
        if ((flags & MF_USE_OCCLUSION_MAP) != 0u)
        {
            float occ = gOcclusion.Sample(gSampler, uv0).r;
            ao = lerp(1.0f, occ, saturate(occlusionStrength));
        }
        if ((flags & MF_USE_ROUGHNESS_MAP) != 0u)
        {
            roughness = max(gRoughness.Sample(gSampler, uv0).r, ROUGHNESS_MIN);
        }
        if ((flags & MF_USE_METALLIC_MAP) != 0u)
        {
            metallic = saturate(gMetallic.Sample(gSampler, uv0).r);
        }
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
            shadowFactor = CalculateShadowFactor(input.positionLightSpace, N, shadowLightDir);

        float NdotL = max(dot(N, -lightDir), 0.0f);

        if (!usePBR)
        {
            diffuseAccum += lights[i].lightDiffuseColor.rgb * NdotL * lightIntensity * shadowFactor;

            float3 halfVec = normalize(-lightDir + viewDir);
            float spec = pow(max(dot(N, halfVec), 0.0f), max(shininess, 1.0f));
            specularAccum += gSpecularColor.rgb * spec
                            * lights[i].lightDiffuseColor.rgb
                            * lightIntensity * shadowFactor * NdotL;
        }
        else
        {
            // PBR Direct Lighting (Metallic/Roughness)
            float3 V = viewDir;
            float3 L = normalize(-lightDir);
            float3 H = normalize(V + L);

            float NdotV = max(dot(N, V), 0.0f);
            float NdotH = max(dot(N, H), 0.0f);
            float VdotH = max(dot(V, H), 0.0f);

            // BaseColor wird spaeter aus Textur*BaseColor*VertexColor gebaut.
            // Hier nur BRDF-Skalar-Anteile (F0 / energy split). Albedo kommt im Final-PBR zusammensetzen.
            // Dielektrik-F0: 0.04 (RGB)
            float3 dielectricF0 = float3(0.04f, 0.04f, 0.04f);

            // Optional: specularColor als "Legacy Bridge" (stark geklemmt), damit alte Materialien nicht voellig unbrauchbar sind.
            float3 legacyF0 = saturate(gSpecularColor.rgb);
            legacyF0 = clamp(legacyF0, 0.02f, 0.08f);

            // Metallic mix: F0 -> BaseColor
            float3 F0 = lerp(dielectricF0, albedo, metallic);

            // Legacy Bridge leicht einmischen (konservativ)
            F0 = lerp(F0, legacyF0, 0.20f);

            // Wir benutzen hier nur den Faktor; echte BaseColor kommt spaeter.
            // (BaseColor -> F0 Mix erfolgt im finalen PBR-Term)

            float D = DistributionGGX(NdotH, roughness);
            float G = GeometrySmith(NdotV, NdotL, roughness);

            // Fresnel braucht echtes F0 (inkl. Metallic-Mix). Das machen wir spaeter mit BaseColor.
            // Hier berechnen wir Fresnel mit F0 und VdotH; Metallic-Mix wird in Final-PBR gemacht.
            float3 F = FresnelSchlick(VdotH, F0);

            float3 numerator = D * G * F;
            float denom = max(4.0f * NdotV * NdotL, 1e-6f);
            float3 specBRDF = numerator / denom;

            // Wir akkumulieren hier diffuse/spec getrennt als "Lighting Weights".
            // Diffuse Gewicht wird im Final-PBR mit BaseColor/PI multipliziert.
            // Spec Gewicht wird im Final-PBR mit echtem F0 (inkl Metallic-Mix) sauber umgesetzt.
            diffuseAccum += lights[i].lightDiffuseColor.rgb * (NdotL * lightIntensity * shadowFactor);

            specularAccum += specBRDF * lights[i].lightDiffuseColor.rgb
                             * (NdotL * lightIntensity * shadowFactor);
        }
    }

    float3 lighting = ((flags & MF_UNLIT) != 0u)
        ? float3(1, 1, 1)
        : (usePBR ? float3(1, 1, 1) : saturate(ambient * ao + diffuseAccum + specularAccum));

    // ── Finale Farbe ──────────────────────────────────────────────────────────
    float3 final_color;
    if (!usePBR)
    {
        final_color = albedo * lighting;
    }
    else
    {
        // PBR Final (Direct + simple ambient replacement)
        // Energy split (approx): kS ~ F0, kD = (1-kS)*(1-metallic)
        // albedo enthaelt bereits texColor (inkl. Detail/Blend-Map) * baseColor * vertexColor.
        // F0 muss albedo benutzen damit Texturen und Detail Maps auch bei Metall sichtbar sind.
        float3 dielectricF0 = float3(0.04f, 0.04f, 0.04f);
        float3 legacyF0 = saturate(gSpecularColor.rgb);
        legacyF0 = clamp(legacyF0, 0.02f, 0.08f);
        float3 F0 = lerp(dielectricF0, albedo, metallic);
        F0 = lerp(F0, legacyF0, 0.20f);

        float3 kS = saturate(F0);
        float3 kD = (1.0f - kS) * (1.0f - metallic);

        // Diffuse (Lambert)
        float3 diffuseTerm = (kD * albedo / PI) * diffuseAccum;

        // SpecularTerm wurde bereits als BRDF*Light akkumuliert
        float3 specularTerm = specularAccum;

        // Ambient: diffuser Anteil nur fuer Nicht-Metalle (kD nahe 0 bei Metall)
        float3 ambientDiffuse = kD * albedo * ambient * ao;

        // Fake Specular Ambient (IBL-Ersatz): Umgebungslicht wird auch im Specular-Kanal
        // sichtbar, damit metallische Objekte bei niedriger Roughness nicht schwarz wirken.
        // Fresnel-Term bei 90 Grad (konservative Naeherung fuer Hemisphere-Integral).
        float3 F_ambient = FresnelSchlick(max(dot(N, viewDir), 0.0f), F0);
        // Rauere Oberflaechen streuen mehr → Specular-Ambient mit Roughness-Daempfung
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