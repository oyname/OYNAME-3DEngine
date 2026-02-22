#include "Material.h"

Material::Material() :
    isActive(false),
    m_texture{},
    m_textureView{},
    m_imageSamplerState{},
    materialBuffer(nullptr),
    pRenderShader(nullptr)
{
    // Defaults
    properties.diffuseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    properties.specularColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    properties.shininess = 32.0f;
    properties.transparency = 1.0f;
    properties.blendMode = 0.0f;
    properties.blendFactor = 1.0f;  // Standard: voller Einfluss

    // Shadow flags default
    castShadows = true;
    receiveShadows = true;
    properties.receiveShadows = 1.0f;
}

Material::~Material() {

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
    ID3D11DeviceContext* ctx = device->GetDeviceContext();

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

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = context->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        memcpy(mapped.pData, &properties, sizeof(properties));
        context->Unmap(materialBuffer, 0);
    }

    context->PSSetConstantBuffers(2, 1, &materialBuffer);
}

void Material::SetDiffuseColor(float r, float g, float b, float a)
{
    properties.diffuseColor = DirectX::XMFLOAT4(r, g, b, a);
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
    properties.transparency = transparency;
}

void Material::SetColor(float r, float g, float b, float a)
{
    SetDiffuseColor(r, g, b, a); 
}

// ==================== GETTERS ====================

DirectX::XMFLOAT4 Material::GetDiffuseColor() const
{
    return properties.diffuseColor;
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