#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "gdxutil.h"
#include "Texture.h"

// Vorwaertsdeklarationen - kein <d3d11.h> im Header.
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;

// ============================================================
//  TexturePool  --  vereinter Textur-Manager
//
//  Ersetzt TextureManager und den alten TexturePool.
//  Aufgaben:
//    - Texturen vom Disk laden (stb_image via Texture)
//    - Duplikate per Dateiname vermeiden
//    - Besitz der Texture-Objekte
//    - Jedem SRV einen stabilen uint32_t-Index zuweisen
//    - Default-Fallback-Texturen bereitstellen
// ============================================================
class TexturePool
{
public:
    TexturePool()  = default;
    ~TexturePool();

    TexturePool(const TexturePool&)            = delete;
    TexturePool& operator=(const TexturePool&) = delete;

    // Laedt eine Textur vom Disk. Bei bereits geladener Datei
    // wird das gecachte Objekt zurueckgegeben.
    HRESULT LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
                        const wchar_t* filename, LPLPTEXTURE lpTexture);

    // Erstellt Default-Fallback-Texturen (weiss, Flat-Normal, ORM).
    // Muss einmalig nach Device-Init aufgerufen werden.
    bool InitializeDefaults(ID3D11Device* device);

    // Registriert einen externen SRV und gibt seinen Pool-Index zurueck.
    uint32_t GetOrAdd(ID3D11ShaderResourceView* srv);

    // Gibt den SRV fuer einen Index zurueck (nullptr bei ungueltigem Index).
    ID3D11ShaderResourceView* GetSRV(uint32_t index) const;

    uint32_t Size()            const { return static_cast<uint32_t>(m_srvs.size()); }
    uint32_t WhiteIndex()      const { return m_whiteIndex;      }
    uint32_t FlatNormalIndex() const { return m_flatNormalIndex; }
    uint32_t OrmIndex()        const { return m_ormIndex;        }

private:
    uint32_t AddInternal(ID3D11ShaderResourceView* srv);

    bool Create1x1TextureSRV(ID3D11Device* device,
                              uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                              ID3D11ShaderResourceView** outSrv);

    int FindByFilename(const std::wstring& filename) const;

    std::vector<ID3D11ShaderResourceView*>                  m_srvs;
    std::unordered_map<ID3D11ShaderResourceView*, uint32_t> m_indexBySrv;
    std::vector<Texture*>                                   m_textures;

    uint32_t m_whiteIndex      = 0;
    uint32_t m_flatNormalIndex = 0;
    uint32_t m_ormIndex        = 0;
    bool     m_defaultsReady   = false;
};
