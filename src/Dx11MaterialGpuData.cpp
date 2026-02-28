// Dx11MaterialGpuData.cpp: alle DX11-Aufrufe gehoeren hierher.
#include <d3d11.h>
#include <DirectXMath.h>
#include "gdxutil.h"
#include "gdxdevice.h"
#include "Dx11MaterialGpuData.h"

MaterialGpuData::MaterialGpuData()
{
}

MaterialGpuData::~MaterialGpuData()
{
    Memory::SafeRelease(materialBuffer);

    for (int i = 0; i < MAX_TEXTURES; ++i)
    {
        Memory::SafeRelease(m_imageSamplerState[i]);
        Memory::SafeRelease(m_textureView[i]);
        Memory::SafeRelease(m_texture[i]);

        m_imageSamplerState[i] = nullptr;
        m_textureView[i]       = nullptr;
        m_texture[i]           = nullptr;
    }
}

void MaterialGpuData::SetTexture(const GDXDevice* device)
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

void MaterialGpuData::SetTexture(int slot,
    ID3D11Texture2D*          texture,
    ID3D11ShaderResourceView* textureView,
    ID3D11SamplerState*       sampler)
{
    if (slot < 0 || slot >= MAX_TEXTURES) return;

    Memory::SafeRelease(m_imageSamplerState[slot]);
    Memory::SafeRelease(m_textureView[slot]);
    Memory::SafeRelease(m_texture[slot]);

    m_texture[slot]           = texture;
    m_textureView[slot]       = textureView;
    m_imageSamplerState[slot] = sampler;

    if (m_texture[slot])           m_texture[slot]->AddRef();
    if (m_textureView[slot])       m_textureView[slot]->AddRef();
    if (m_imageSamplerState[slot]) m_imageSamplerState[slot]->AddRef();
}

void MaterialGpuData::UpdateConstantBuffer(
    ID3D11DeviceContext* context,
    const DirectX::XMFLOAT4&   baseColor,
    const DirectX::XMFLOAT4&   specularColor,
    const DirectX::XMFLOAT4&   emissiveColor,
    const DirectX::XMFLOAT4&   uvTilingOffset,
    float metallic, float roughness, float normalScale, float occlusionStrength,
    float shininess, float transparency, float alphaCutoff, float receiveShadows,
    uint32_t albedoIndex, uint32_t normalIndex, uint32_t ormIndex, uint32_t decalIndex,
    float blendMode, uint32_t flags)
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

        DirectX::XMUINT4  texIndex;
        DirectX::XMUINT4  misc;
    };

    static_assert(sizeof(MaterialCB) == 128, "MaterialCB must be 128 bytes");

    auto clampIdx = [](uint32_t v, uint32_t fallback) -> uint32_t {
        return (v <= 15u) ? v : fallback;
    };

    // GDX::Float4 -> DirectX::XMFLOAT4: identisches Memory-Layout, direkte Member-Kopie.
    MaterialCB cb{};
    cb.baseColor      = { baseColor.x,      baseColor.y,      baseColor.z,      baseColor.w      };
    cb.specularColor  = { specularColor.x,  specularColor.y,  specularColor.z,  specularColor.w  };
    cb.emissiveColor  = { emissiveColor.x,  emissiveColor.y,  emissiveColor.z,  emissiveColor.w  };
    cb.uvTilingOffset = { uvTilingOffset.x, uvTilingOffset.y, uvTilingOffset.z, uvTilingOffset.w };

    cb.pbr   = { metallic, roughness, normalScale, occlusionStrength };
    cb.alpha = { shininess, transparency, alphaCutoff, receiveShadows };

    cb.texIndex = DirectX::XMUINT4(
        clampIdx(albedoIndex, 0u),
        clampIdx(normalIndex, 1u),
        clampIdx(ormIndex,    2u),
        clampIdx(decalIndex,  0u)
    );

    cb.misc = DirectX::XMUINT4(
        static_cast<uint32_t>(blendMode + 0.5f),
        static_cast<uint32_t>(flags),
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
