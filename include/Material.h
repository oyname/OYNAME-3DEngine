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
    struct MaterialData {
        DirectX::XMFLOAT4 diffuseColor;
        DirectX::XMFLOAT4 specularColor;
        float shininess;
        float transparency;
        float receiveShadows; // 1.0 = receive, 0.0 = ignore
        float blendMode;      // 0=off 1=multiply 2=multiply�2 3=additive 4=lerp(alpha) 5=luminanz
        float blendFactor;      
        float padding[3];      
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

    inline void SetBlendFactor(float f) { properties.blendFactor = f; }
    inline float GetBlendFactor() const { return properties.blendFactor; }

    // ==================== MATERIAL PROPERTY GETTERS ====================
    DirectX::XMFLOAT4 GetDiffuseColor() const;
    DirectX::XMFLOAT4 GetSpecularColor() const;
    float GetShininess() const;
    float GetTransparency() const;

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

