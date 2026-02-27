#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <d3d11.h>

class TexturePool
{
public:
    TexturePool() = default;
    ~TexturePool();

    TexturePool(const TexturePool&) = delete;
    TexturePool& operator=(const TexturePool&) = delete;

    bool InitializeDefaults(ID3D11Device* device);

    uint32_t GetOrAdd(ID3D11ShaderResourceView* srv);
    ID3D11ShaderResourceView* GetSRV(uint32_t index) const;

    uint32_t Size() const { return static_cast<uint32_t>(m_srvs.size()); }

    uint32_t WhiteIndex()      const { return m_whiteIndex; }
    uint32_t FlatNormalIndex() const { return m_flatNormalIndex; }
    uint32_t OrmIndex()        const { return m_ormIndex; }

private:
    uint32_t AddInternal(ID3D11ShaderResourceView* srv);
    bool Create1x1TextureSRV(
        ID3D11Device* device,
        uint8_t r, uint8_t g, uint8_t b, uint8_t a,
        ID3D11ShaderResourceView** outSrv);

private:
    std::vector<ID3D11ShaderResourceView*> m_srvs;
    std::unordered_map<ID3D11ShaderResourceView*, uint32_t> m_indexBySrv;

    uint32_t m_whiteIndex = 0;
    uint32_t m_flatNormalIndex = 0;
    uint32_t m_ormIndex = 0;

    bool m_defaultsReady = false;
};
