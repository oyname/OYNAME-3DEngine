#pragma once
#include <string>
#include <cstdint>
#include <DirectXMath.h>

// Material.h kennt kein DX11 (keine COM-Interfaces, kein <d3d11.h>).
// DirectXMath bleibt – es ist eine reine Mathematik-Bibliothek.
// Alle GPU-Ressourcen (Texturen, Sampler, Constant Buffer) leben in MaterialGpuData.
class MaterialGpuData;
class Shader;
class Mesh;

class Material
{
public:
    // ==================== MATERIAL DATA STRUCT ====================
    // Must match PixelShader.hlsl cbuffer MaterialBuffer (b2)
    // NOTE:
    // - Layout is 16-byte aligned (DX11 constant buffer requirement).
    struct MaterialData {
        // --- Classic ---
        DirectX::XMFLOAT4 baseColor;       // (legacy: diffuseColor)
        DirectX::XMFLOAT4 specularColor;

        // --- Extras (optional, shader decides) ---
        DirectX::XMFLOAT4 emissiveColor;   // rgb * intensity
        DirectX::XMFLOAT4 uvTilingOffset;  // xy = tiling, zw = offset

        // pack 16 bytes (PBR)
        float metallic;          // 0..1
        float roughness;         // 0..1
        float normalScale;       // 0..2
        float occlusionStrength; // 0..1

        // pack 16 bytes
        float shininess;         // legacy phong
        float transparency;      // 0..1 (legacy)
        float alphaCutoff;       // 0..1 (alpha test)
        float receiveShadows;    // 1.0 = receive, 0.0 = ignore

        // pack 16 bytes
        float    blendMode;      // 0=off 1=multiply 2=multiply*2 3=additive 4=lerp(alpha) 5=luminance
        float    blendFactor;    // used by blendMode in PS
        uint32_t flags;          // see MaterialFlags
        float    _pad0;          // padding
    };

    enum MaterialFlags : uint32_t
    {
        MF_NONE = 0,
        MF_ALPHA_TEST = 1u << 0,
        MF_DOUBLE_SIDED = 1u << 1,
        MF_UNLIT = 1u << 2,
        MF_USE_NORMAL_MAP = 1u << 3,
        MF_USE_ORM_MAP = 1u << 4,
        MF_USE_EMISSIVE = 1u << 5,
        MF_TRANSPARENT = 1u << 6,

        MF_USE_OCCLUSION_MAP = 1u << 7,
        MF_USE_ROUGHNESS_MAP = 1u << 8,
        MF_USE_METALLIC_MAP = 1u << 9,

        // ===== PBR Shading Mode =====
        MF_SHADING_PBR = 1u << 10
    };

    // ==================== KONSTRUKTOR / DESTRUKTOR ====================
    Material();
    ~Material();

    // ==================== MATERIAL PROPERTY SETTERS ====================
    void SetDiffuseColor(float r, float g, float b, float a = 1.0f);
    void SetSpecularColor(float r, float g, float b, float a = 1.0f);
    void SetShininess(float shininess);
    void SetTransparency(float transparency);
    void SetColor(float r, float g, float b, float a = 1.0f);

    void SetMetallic(float m);
    void SetRoughness(float r);
    void SetNormalScale(float s);
    void SetOcclusionStrength(float s);
    void SetEmissiveColor(float r, float g, float b, float intensity = 1.0f);
    void SetUVTilingOffset(float tileU, float tileV, float offU = 0.0f, float offV = 0.0f);
    void SetAlphaCutoff(float cutoff);

    inline void SetBlendFactor(float f) { properties.blendFactor = f; }
    inline float GetBlendFactor() const { return properties.blendFactor; }

    inline void SetAlphaTest(bool enabled)
    {
        if (enabled) properties.flags |= MF_ALPHA_TEST;
        else         properties.flags &= ~MF_ALPHA_TEST;
    }
    inline bool IsAlphaTest() const { return (properties.flags & MF_ALPHA_TEST) != 0; }

    inline void SetTransparent(bool enabled)
    {
        if (enabled) properties.flags |= MF_TRANSPARENT;
        else         properties.flags &= ~MF_TRANSPARENT;
    }
    inline bool IsTransparent() const { return (properties.flags & MF_TRANSPARENT) != 0; }

    inline void SetDoubleSided(bool enabled)
    {
        if (enabled) properties.flags |= MF_DOUBLE_SIDED;
        else         properties.flags &= ~MF_DOUBLE_SIDED;
    }
    inline bool IsDoubleSided() const { return (properties.flags & MF_DOUBLE_SIDED) != 0; }

    // Stable numeric identifier assigned by AssetManager::CreateMaterial.
    // Wird in RenderQueue::Sort() als SortKey genutzt (ersetzt Pointer-Truncation).
    // 0 = nicht initialisiert (Material nicht ueber CreateMaterial erstellt).
    uint32_t id = 0;

    uint32_t albedoIndex = 0;
    uint32_t normalIndex = 1;
    uint32_t ormIndex = 2;
    uint32_t decalIndex = 0;

    // ==================== NEU (Schritt 2): Separate PBR Texture Indizes ====================
    // Default 0: "white" (AO=1, Roughness=1 wenn Kanal so interpretiert wird)
    // Metallic default: bleibt über scalar metallic steuerbar, Flag bleibt aus, bis Textur gesetzt wird.
    uint32_t occlusionIndex = 0;
    uint32_t roughnessIndex = 0;
    uint32_t metallicIndex = 0;

    DirectX::XMFLOAT4 GetDiffuseColor() const;
    DirectX::XMFLOAT4 GetSpecularColor() const;
    float GetShininess() const;
    float GetTransparency() const;

    float             GetMetallic()          const { return properties.metallic; }
    float             GetRoughness()         const { return properties.roughness; }
    float             GetNormalScale()       const { return properties.normalScale; }
    float             GetOcclusionStrength() const { return properties.occlusionStrength; }
    DirectX::XMFLOAT4 GetEmissiveColor()     const { return properties.emissiveColor; }
    DirectX::XMFLOAT4 GetUVTilingOffset()    const { return properties.uvTilingOffset; }
    float             GetAlphaCutoff()       const { return properties.alphaCutoff; }

    void SetAlbedoIndex(uint32_t idx) noexcept { albedoIndex = idx; }
    void SetNormalIndex(uint32_t idx) noexcept
    {
        normalIndex = idx;
        if (idx != 1u) properties.flags |= MF_USE_NORMAL_MAP;
        else           properties.flags &= ~MF_USE_NORMAL_MAP;
    }
    void SetOrmIndex(uint32_t idx) noexcept
    {
        ormIndex = idx;
        if (idx != 2u) properties.flags |= MF_USE_ORM_MAP;
        else           properties.flags &= ~MF_USE_ORM_MAP;
    }
    void SetDecalIndex(uint32_t idx) noexcept { decalIndex = idx; }

    // ==================== NEU (Schritt 2): Separate PBR Setter ====================
    inline void SetOcclusionIndex(uint32_t idx) noexcept
    {
        occlusionIndex = idx;
        if (idx != 0u) properties.flags |= MF_USE_OCCLUSION_MAP;
        else           properties.flags &= ~MF_USE_OCCLUSION_MAP;
    }

    inline void SetRoughnessIndex(uint32_t idx) noexcept
    {
        roughnessIndex = idx;
        if (idx != 0u) properties.flags |= MF_USE_ROUGHNESS_MAP;
        else           properties.flags &= ~MF_USE_ROUGHNESS_MAP;
    }

    inline void SetMetallicIndex(uint32_t idx) noexcept
    {
        metallicIndex = idx;
        if (idx != 0u) properties.flags |= MF_USE_METALLIC_MAP;
        else           properties.flags &= ~MF_USE_METALLIC_MAP;
    }

    
    // ==================== SHADING MODE ====================
    inline void SetUsePBR(bool enabled) noexcept
    {
        if (enabled) properties.flags |= MF_SHADING_PBR;
        else         properties.flags &= ~MF_SHADING_PBR;
    }
    inline bool GetUsePBR() const noexcept { return (properties.flags & MF_SHADING_PBR) != 0u; }

// ==================== SHADOW FLAGS ====================
    // Getter/Setter – direkte Feldzugriffe vermeiden, da receiveShadows
    // mit properties.receiveShadows (float fuer den Shader) synchron bleiben muss.
    inline void SetCastShadows(bool enabled)    { m_castShadows = enabled; }
    inline void SetReceiveShadows(bool enabled)
    {
        m_receiveShadows = enabled;
        properties.receiveShadows = enabled ? 1.0f : 0.0f;
    }
    inline bool GetCastShadows()    const { return m_castShadows; }
    inline bool GetReceiveShadows() const { return m_receiveShadows; }

    // ==================== BLEND MODE ====================
    inline void SetBlendMode(int mode) { properties.blendMode = (float)mode; }
    inline int  GetBlendMode()   const { return (int)properties.blendMode; }

    // ==================== MATERIAL STATE ====================
    bool         isActive;
    MaterialData properties;

    // ==================== GPU DATA ====================
    // Alle DX11-Ressourcen ausgelagert.
    // Zugriff ueber material->gpuData->...
    MaterialGpuData* gpuData = nullptr;

    // ==================== OBJECT MANAGEMENT ====================
    Shader* pRenderShader;

    // ==================== MEMORY MANAGEMENT ====================
    void* operator new(size_t size) {
        return _aligned_malloc(size, 16);
    }
    void operator delete(void* p) noexcept {
        _aligned_free(p);
    }

private:
    // Shadow-Flags als private Member – Zugriff nur ueber Setter,
    // damit properties.receiveShadows (float) stets synchron bleibt.
    bool m_castShadows    = true;
    bool m_receiveShadows = true;
};

typedef Material* LPMATERIAL;
typedef Material  MATERIAL;