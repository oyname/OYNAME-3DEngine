#pragma once
#include <cstdint>
#include <DirectXMath.h>

// Vorwaertsdeklarationen – kein <d3d11.h> im Header noetig.
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11Buffer;
struct ID3D11DeviceContext;
class  GDXDevice;


// MaterialGpuData haelt alle DX11-Ressourcen, die zu einem Material gehoeren:
// Textur-Slots, Sampler und den Constant Buffer. Material selbst kennt
// diese Typen nicht mehr.
class MaterialGpuData
{
public:
    static const int MAX_TEXTURES = 8;

    MaterialGpuData();
    ~MaterialGpuData();

    // Setzt alle Textur-Slots des Slots auf den Pixel-Shader.
    void SetTexture(const GDXDevice* device);

    // Belegt einen einzelnen Slot mit Textur, SRV und Sampler.
    void SetTexture(int slot,
        ID3D11Texture2D*          texture,
        ID3D11ShaderResourceView* textureView,
        ID3D11SamplerState*       sampler);

    // Schreibt MaterialData in den Constant Buffer und bindet ihn an b2.
    void UpdateConstantBuffer(
        ID3D11DeviceContext* context,
        const DirectX::XMFLOAT4&   baseColor,
        const DirectX::XMFLOAT4&   specularColor,
        const DirectX::XMFLOAT4&   emissiveColor,
        const DirectX::XMFLOAT4&   uvTilingOffset,
        float metallic, float roughness, float normalScale, float occlusionStrength,
        float shininess, float transparency, float alphaCutoff, float receiveShadows,
        uint32_t albedoIndex, uint32_t normalIndex, uint32_t ormIndex, uint32_t decalIndex,
        float blendMode, uint32_t flags);

    ID3D11Texture2D*          m_texture[MAX_TEXTURES]          = {};
    ID3D11ShaderResourceView* m_textureView[MAX_TEXTURES]       = {};
    ID3D11SamplerState*       m_imageSamplerState[MAX_TEXTURES] = {};

    ID3D11Buffer* materialBuffer = nullptr;
};
