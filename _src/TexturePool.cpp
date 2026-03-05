// TexturePool.cpp: vereinter Textur-Manager.
// Kein doppeltes System mehr -- Laden, Caching, Index-Vergabe und
// Default-Texturen laufen hier zusammen.
#include <d3d11.h>
#include "TexturePool.h"
#include "Texture.h"
#include "gdxutil.h"

// ============================================================
TexturePool::~TexturePool()
{
    for (ID3D11ShaderResourceView* srv : m_srvs)
        if (srv) srv->Release();

    m_srvs.clear();
    m_indexBySrv.clear();

    for (Texture* t : m_textures)
        Memory::SafeDelete(t);

    m_textures.clear();
}

// ============================================================
//  LoadTexture
//
//  Prueft per Dateiname ob die Textur bereits geladen ist.
//  Wenn ja: gecachtes Texture-Objekt zurueckgeben.
//  Wenn nein: laden, SRV im Pool registrieren, speichern.
// ============================================================
HRESULT TexturePool::LoadTexture(ID3D11Device* device,
                                  ID3D11DeviceContext* deviceContext,
                                  const wchar_t* filename,
                                  LPLPTEXTURE lpTexture)
{
    if (!lpTexture)
        return E_INVALIDARG;

    *lpTexture = nullptr;

    int existing = FindByFilename(filename);
    if (existing >= 0)
    {
        *lpTexture = m_textures[existing];
        Debug::Log("texturepool.cpp: Textur aus Cache zurueckgegeben: ",
            std::wstring(filename).c_str());
        return S_OK;
    }

    Texture* tex = new Texture;
    HRESULT hr = tex->AddTexture(device, deviceContext, filename);
    if (FAILED(hr))
    {
        Debug::Log("texturepool.cpp: Laden fehlgeschlagen: ",
            std::wstring(filename).c_str());
        Memory::SafeDelete(tex);
        return hr;
    }

    // SRV im Pool registrieren
    if (tex->m_textureView)
        GetOrAdd(tex->m_textureView);

    m_textures.push_back(tex);
    *lpTexture = tex;

    Debug::Log("texturepool.cpp: Textur geladen (Pool-Groesse: ",
        static_cast<int>(m_srvs.size()), ")");
    return S_OK;
}

// ============================================================
int TexturePool::FindByFilename(const std::wstring& filename) const
{
    for (int i = 0; i < static_cast<int>(m_textures.size()); ++i)
        if (m_textures[i] && m_textures[i]->m_sFilename == filename)
            return i;
    return -1;
}

// ============================================================
uint32_t TexturePool::GetOrAdd(ID3D11ShaderResourceView* srv)
{
    if (!m_defaultsReady) return 0;
    if (!srv) return m_whiteIndex;

    auto it = m_indexBySrv.find(srv);
    if (it != m_indexBySrv.end())
        return it->second;

    return AddInternal(srv);
}

// ============================================================
uint32_t TexturePool::AddInternal(ID3D11ShaderResourceView* srv)
{
    if (!srv) return m_whiteIndex;

    const uint32_t idx = static_cast<uint32_t>(m_srvs.size());
    srv->AddRef();
    m_srvs.push_back(srv);
    m_indexBySrv[srv] = idx;
    return idx;
}

// ============================================================
ID3D11ShaderResourceView* TexturePool::GetSRV(uint32_t index) const
{
    if (index >= m_srvs.size()) return nullptr;
    return m_srvs[index];
}

// ============================================================
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
    Debug::Log("texturepool.cpp: Default-Texturen erstellt (weiss=",
        m_whiteIndex, " flatNormal=", m_flatNormalIndex,
        " orm=", m_ormIndex, ")");
    return true;
}

// ============================================================
bool TexturePool::Create1x1TextureSRV(ID3D11Device* device,
                                       uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                                       ID3D11ShaderResourceView** outSrv)
{
    if (!device || !outSrv) return false;
    *outSrv = nullptr;

    const uint32_t pixel =
        (uint32_t)a << 24u | (uint32_t)b << 16u | (uint32_t)g << 8u | (uint32_t)r;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width            = 1;
    desc.Height           = 1;
    desc.MipLevels        = 1;
    desc.ArraySize        = 1;
    desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage            = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA init{};
    init.pSysMem     = &pixel;
    init.SysMemPitch = sizeof(uint32_t);

    ID3D11Texture2D* tex = nullptr;
    HRESULT hr = device->CreateTexture2D(&desc, &init, &tex);
    if (FAILED(hr) || !tex) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format                    = desc.Format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels       = 1;

    hr = device->CreateShaderResourceView(tex, &srvDesc, outSrv);
    tex->Release();

    return SUCCEEDED(hr) && *outSrv;
}
