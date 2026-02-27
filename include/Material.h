#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include "gdxutil.h"
#include "Memory.h"

// Forward declarations – Mesh.h wird NICHT inkludiert (verhindert zirkuläre Abhängigkeit)
class Shader;
class Mesh;
class GDXDevice;

class Material
{
public:
    // ==================== MATERIAL DATA STRUCT ====================
    // Must match PixelShader.hlsl cbuffer MaterialBuffer (b2)
    // NOTE:
    // - Layout is 16-byte aligned (DX11 constant buffer requirement).
    // - Existing fields (diffuse/specular/shininess/transparency/blend*) remain,
    //   but the struct is extended with common material properties.
    struct MaterialData {
        // --- Classic ---
        DirectX::XMFLOAT4 baseColor;       // (legacy: diffuseColor)
        DirectX::XMFLOAT4 specularColor;

        // --- Extras (optional, shader decides) ---
        DirectX::XMFLOAT4 emissiveColor;   // rgb * intensity
        DirectX::XMFLOAT4 uvTilingOffset;  // xy = tiling, zw = offset

        // pack 16 bytes
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
        MF_NONE            = 0,
        MF_ALPHA_TEST      = 1u << 0, // discard if alpha < alphaCutoff
        MF_DOUBLE_SIDED    = 1u << 1, // cull off (needs pipeline state)
        MF_UNLIT           = 1u << 2, // ignore lights (shader side)
        MF_USE_NORMAL_MAP  = 1u << 3,
        MF_USE_ORM_MAP     = 1u << 4, // Occlusion/Roughness/Metallic packed
        MF_USE_EMISSIVE    = 1u << 5,
    };

    // ==================== KONSTRUKTOR / DESTRUKTOR ====================
    Material();
    ~Material();

    // ==================== TEXTURE METHODS ====================
    void SetTexture(const GDXDevice* device);
    void SetTexture(int slot, ID3D11Texture2D* texture, ID3D11ShaderResourceView* textureView, ID3D11SamplerState* imageSamplerState);
    void UpdateConstantBuffer(ID3D11DeviceContext* context);

    // ==================== MATERIAL PROPERTY SETTERS ====================
    void SetDiffuseColor(float r, float g, float b, float a = 1.0f);
    void SetSpecularColor(float r, float g, float b, float a = 1.0f);
    void SetShininess(float shininess);
    void SetTransparency(float transparency);
    void SetColor(float r, float g, float b, float a = 1.0f);  // Kurzform fuer Diffuse

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

    inline void SetDoubleSided(bool enabled)
    {
        if (enabled) properties.flags |= MF_DOUBLE_SIDED;
        else         properties.flags &= ~MF_DOUBLE_SIDED;
    }
    inline bool IsDoubleSided() const { return (properties.flags & MF_DOUBLE_SIDED) != 0; }

    // ==================== MATERIAL PROPERTY GETTERS ====================
    DirectX::XMFLOAT4 GetDiffuseColor() const;
    DirectX::XMFLOAT4 GetSpecularColor() const;
    float GetShininess() const;
    float GetTransparency() const;

    float GetMetallic() const { return properties.metallic; }
    float GetRoughness() const { return properties.roughness; }
    float GetNormalScale() const { return properties.normalScale; }
    float GetOcclusionStrength() const { return properties.occlusionStrength; }
    DirectX::XMFLOAT4 GetEmissiveColor() const { return properties.emissiveColor; }
    DirectX::XMFLOAT4 GetUVTilingOffset() const { return properties.uvTilingOffset; }
    float GetAlphaCutoff() const { return properties.alphaCutoff; }

    // ==================== SHADOW FLAGS ====================
    // CPU-side flags
    bool castShadows = true;
    bool receiveShadows = true;

    inline void SetCastShadows(bool enabled) { castShadows = enabled; } // material->SetCastShadows(false);     // wirft keinen Schatten
    inline void SetReceiveShadows(bool enabled)                         // material->SetReceiveShadows(false);  // wird nicht abgedunkelt durch ShadowMap
    {
        receiveShadows = enabled;
        properties.receiveShadows = enabled ? 1.0f : 0.0f;
    }
    inline bool GetCastShadows() const { return castShadows; }
    inline bool GetReceiveShadows() const { return receiveShadows; }

    // ==================== BLEND MODE ====================
    inline void SetBlendMode(int mode)
    {
        properties.blendMode = (float)mode;
    }
    inline int GetBlendMode() const { return (int)properties.blendMode; }

    // ==================== MATERIAL STATE ====================
    bool isActive;
    MaterialData properties;  // Alle Material-Properties hier!

    // ==================== TEXTURE DATA ====================
    static const int MAX_TEXTURES = 8;

    ID3D11Texture2D* m_texture[MAX_TEXTURES] = {};
    ID3D11ShaderResourceView* m_textureView[MAX_TEXTURES] = {};
    ID3D11SamplerState* m_imageSamplerState[MAX_TEXTURES] = {};

    ID3D11Buffer* materialBuffer;  // Constant Buffer fuer GPU

    // ==================== OBJECT MANAGEMENT ====================
    Shader* pRenderShader;

    // ==================== MEMORY MANAGEMENT ====================
    void* operator new(size_t size) {
        return _aligned_malloc(size, 16);
    }
    void operator delete(void* p) noexcept {
        _aligned_free(p);
    }
};

typedef Material* LPMATERIAL;
typedef Material MATERIAL;
