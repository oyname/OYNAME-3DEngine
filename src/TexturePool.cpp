#include "TexturePool.h"

TexturePool::~TexturePool()
{
    for (ID3D11ShaderResourceView* srv : m_srvs)
        if (srv) srv->Release();

    m_srvs.clear();
    m_indexBySrv.clear();
}

bool TexturePool::Create1x1TextureSRV(
    ID3D11Device* device,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a,
    ID3D11ShaderResourceView** outSrv)
{
    if (!device || !outSrv) return false;
    *outSrv = nullptr;

    const uint32_t pixel =
        (uint32_t)a << 24u | (uint32_t)b << 16u | (uint32_t)g << 8u | (uint32_t)r;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = 1;
    desc.Height = 1;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem = &pixel;
    init.SysMemPitch = sizeof(uint32_t);

    ID3D11Texture2D* tex = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, &init, &tex);
    if (FAILED(hr) || !tex) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    ID3D11ShaderResourceView* srv = nullptr;
    hr = device->CreateShaderResourceView(tex, &srvDesc, &srv);
    tex->Release();

    if (FAILED(hr) || !srv) return false;
    *outSrv = srv;
    return true;
}

uint32_t TexturePool::AddInternal(ID3D11ShaderResourceView* srv)
{
    if (!srv) return m_whiteIndex;

    const uint32_t idx = static_cast<uint32_t>(m_srvs.size());
    srv->AddRef();
    m_srvs.push_back(srv);
    m_indexBySrv[srv] = idx;
    return idx;
}

bool TexturePool::InitializeDefaults(ID3D11Device* device)
{
    if (m_defaultsReady) return true;
    if (!device) return false;

    ID3D11ShaderResourceView* white = nullptr;
    if (!Create1x1TextureSRV(device, 255, 255, 255, 255, &white)) return false;
    m_whiteIndex = AddInternal(white);
    white->Release();

    ID3D11ShaderResourceView* flatN = nullptr;
    if (!Create1x1TextureSRV(device, 128, 128, 255, 255, &flatN)) return false;
    m_flatNormalIndex = AddInternal(flatN);
    flatN->Release();

    ID3D11ShaderResourceView* orm = nullptr;
    if (!Create1x1TextureSRV(device, 255, 255, 0, 255, &orm)) return false;
    m_ormIndex = AddInternal(orm);
    orm->Release();

    m_defaultsReady = true;
    return true;
}

uint32_t TexturePool::GetOrAdd(ID3D11ShaderResourceView* srv)
{
    if (!m_defaultsReady) return 0; // absichtlich “safe minimal”
    if (!srv) return m_whiteIndex;

    auto it = m_indexBySrv.find(srv);
    if (it != m_indexBySrv.end()) return it->second;

    return AddInternal(srv);
}

ID3D11ShaderResourceView* TexturePool::GetSRV(uint32_t index) const
{
    if (index >= m_srvs.size()) return nullptr;
    return m_srvs[index];
}