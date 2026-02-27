#include "Material.h"
#include "gdxdevice.h"

static float Clamp01(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

Material::Material() :
    isActive(false),
    m_texture{},
    m_textureView{},
    m_imageSamplerState{},
    materialBuffer(nullptr),
    pRenderShader(nullptr)
{
    // Defaults
    properties.baseColor        = DirectX::XMFLOAT4(1, 1, 1, 1);
    properties.specularColor    = DirectX::XMFLOAT4(1, 1, 1, 1);
    properties.emissiveColor    = DirectX::XMFLOAT4(0, 0, 0, 0);
    properties.uvTilingOffset   = DirectX::XMFLOAT4(1, 1, 0, 0);

    properties.metallic          = 0.0f;
    properties.roughness         = 1.0f;
    properties.normalScale       = 1.0f;
    properties.occlusionStrength = 1.0f;

    properties.shininess     = 32.0f;
    properties.transparency  = 1.0f;
    properties.alphaCutoff   = 0.5f;

    properties.blendMode   = 0.0f;
    properties.blendFactor = 1.0f;

    properties.flags = MF_NONE;
    properties._pad0 = 0.0f;

    // Shadow flags default
    castShadows = true;
    receiveShadows = true;
    properties.receiveShadows = 1.0f;
}

Material::~Material()
{
    for (int i = 0; i < MAX_TEXTURES; i++)
    {
        if (m_textureView[i])
        {
            Memory::SafeRelease(m_imageSamplerState[i]);
            Memory::SafeRelease(m_textureView[i]);
            Memory::SafeRelease(m_texture[i]);
        }
    }

    Memory::SafeRelease(materialBuffer);
}

void Material::SetTexture(const GDXDevice* device)
{
    ID3D11DeviceContext* ctx = device ? device->GetDeviceContext() : nullptr;
    if (!ctx) return;

    for (int i = 0; i < MAX_TEXTURES; i++)
    {
        if (m_textureView[i])
        {
            ctx->PSSetShaderResources(i, 1, &m_textureView[i]);
            ctx->PSSetSamplers(i, 1, &m_imageSamplerState[i]);
        }
    }
}

void Material::SetTexture(int slot,
    ID3D11Texture2D* texture,
    ID3D11ShaderResourceView* textureView,
    ID3D11SamplerState* sampler)
{
    if (slot < 0 || slot >= MAX_TEXTURES) return;

    Memory::SafeRelease(m_imageSamplerState[slot]);
    Memory::SafeRelease(m_textureView[slot]);
    Memory::SafeRelease(m_texture[slot]);

    m_texture[slot] = texture;
    m_textureView[slot] = textureView;
    m_imageSamplerState[slot] = sampler;

    if (m_texture[slot])           m_texture[slot]->AddRef();
    if (m_textureView[slot])       m_textureView[slot]->AddRef();
    if (m_imageSamplerState[slot]) m_imageSamplerState[slot]->AddRef();
}

void Material::UpdateConstantBuffer(ID3D11DeviceContext* context)
{
    if (materialBuffer == nullptr || context == nullptr)
        return;

    // GPU-Layout passend zum HLSL cbuffer (b2)
    struct alignas(16) MaterialCB
    {
        DirectX::XMFLOAT4 baseColor;
        DirectX::XMFLOAT4 specularColor;
        DirectX::XMFLOAT4 emissiveColor;
        DirectX::XMFLOAT4 uvTilingOffset;

        DirectX::XMFLOAT4 pbr;    // metallic roughness normalScale occlusionStrength
        DirectX::XMFLOAT4 alpha;  // shininess transparency alphaCutoff receiveShadows

        DirectX::XMUINT4  texIndex; // albedo normal orm decal
        DirectX::XMUINT4  misc;     // blendMode flags 0 0
    };

    static_assert(sizeof(MaterialCB) == 128, "MaterialCB must be 128 bytes");

    auto clampIdx = [](uint32_t v, uint32_t fallback) -> uint32_t {
        return (v <= 15u) ? v : fallback;
        };

    MaterialCB cb{};
    cb.baseColor = properties.baseColor;
    cb.specularColor = properties.specularColor;
    cb.emissiveColor = properties.emissiveColor;
    cb.uvTilingOffset = properties.uvTilingOffset;

    cb.pbr = DirectX::XMFLOAT4(properties.metallic, properties.roughness, properties.normalScale, properties.occlusionStrength);
    cb.alpha = DirectX::XMFLOAT4(properties.shininess, properties.transparency, properties.alphaCutoff, properties.receiveShadows);

    cb.texIndex = DirectX::XMUINT4(
        clampIdx(albedoIndex, 0u), // white
        clampIdx(normalIndex, 1u), // flat normal
        clampIdx(ormIndex, 2u), // default orm
        clampIdx(decalIndex, 0u)
    );

    cb.misc = DirectX::XMUINT4(
        static_cast<uint32_t>(properties.blendMode + 0.5f),
        static_cast<uint32_t>(properties.flags),
        0u, 0u
    );

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = context->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        memcpy(mapped.pData, &cb, sizeof(cb));
        context->Unmap(materialBuffer, 0);
    }

    context->PSSetConstantBuffers(2, 1, &materialBuffer);
}

// ==================== Setters ====================

void Material::SetDiffuseColor(float r, float g, float b, float a)
{
    properties.baseColor = DirectX::XMFLOAT4(r, g, b, a);
}

void Material::SetSpecularColor(float r, float g, float b, float a)
{
    properties.specularColor = DirectX::XMFLOAT4(r, g, b, a);
}

void Material::SetShininess(float shininess)
{
    properties.shininess = shininess;
}

void Material::SetTransparency(float transparency)
{
    properties.transparency = Clamp01(transparency);
}

void Material::SetColor(float r, float g, float b, float a)
{
    SetDiffuseColor(r, g, b, a);
}

void Material::SetMetallic(float m)
{
    properties.metallic = Clamp01(m);
}

void Material::SetRoughness(float r)
{
    properties.roughness = Clamp01(r);
}

void Material::SetNormalScale(float s)
{
    // allow >1 for exaggerated normals
    if (s < 0.0f) s = 0.0f;
    properties.normalScale = s;
    if (s > 0.0f) properties.flags |= MF_USE_NORMAL_MAP;
}

void Material::SetOcclusionStrength(float s)
{
    properties.occlusionStrength = Clamp01(s);
}

void Material::SetEmissiveColor(float r, float g, float b, float intensity)
{
    if (intensity < 0.0f) intensity = 0.0f;
    properties.emissiveColor = DirectX::XMFLOAT4(r * intensity, g * intensity, b * intensity, 0.0f);
    if ((r != 0.0f) || (g != 0.0f) || (b != 0.0f) || (intensity != 0.0f))
        properties.flags |= MF_USE_EMISSIVE;
    else
        properties.flags &= ~MF_USE_EMISSIVE;
}

void Material::SetUVTilingOffset(float tileU, float tileV, float offU, float offV)
{
    properties.uvTilingOffset = DirectX::XMFLOAT4(tileU, tileV, offU, offV);
}

void Material::SetAlphaCutoff(float cutoff)
{
    properties.alphaCutoff = Clamp01(cutoff);
}

// ==================== Getters ====================

DirectX::XMFLOAT4 Material::GetDiffuseColor() const
{
    return properties.baseColor;
}

DirectX::XMFLOAT4 Material::GetSpecularColor() const
{
    return properties.specularColor;
}

float Material::GetShininess() const
{
    return properties.shininess;
}

float Material::GetTransparency() const
{
    return properties.transparency;
}
