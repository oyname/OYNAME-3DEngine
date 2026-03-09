#pragma once

#include <d3d11.h>
#include <cstring>
#include "gdxutil.h"
#include "gdxdevice.h"

struct MatrixSet;

class EntityGpuData
{
public:
    struct FrameStats
    {
        unsigned int uploads = 0;
        unsigned int binds = 0;
        unsigned int ringRotations = 0;
    };

    static constexpr unsigned int kBufferCount = 3;

    EntityGpuData() = default;

    ~EntityGpuData()
    {
        for (unsigned int i = 0; i < kBufferCount; ++i)
            Memory::SafeRelease(m_constantBuffers[i]);
        constantBuffer = nullptr;
    }

    static void ResetFrameStats()
    {
        StatsStorage() = {};
    }

    static FrameStats GetFrameStats()
    {
        return StatsStorage();
    }

    void Upload(const GDXDevice* device, const MatrixSet& matrices)
    {
        if (!device) return;
        if (!EnsureRingBuffers(device)) return;

        const unsigned int nextIndex = (m_currentBufferIndex + 1u) % kBufferCount;
        if (m_constantBuffers[nextIndex])
        {
            m_currentBufferIndex = nextIndex;
            constantBuffer = m_constantBuffers[m_currentBufferIndex];
            ++StatsStorage().ringRotations;
        }

        D3D11_MAPPED_SUBRESOURCE mapped{};
        HRESULT hr = device->GetDeviceContext()->Map(
            constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

        if (FAILED(hr))
        {
            DBLOG_HR(hr);
            return;
        }

        std::memcpy(mapped.pData, &matrices, sizeof(MatrixSet));
        device->GetDeviceContext()->Unmap(constantBuffer, 0);
        Bind(device);
        ++StatsStorage().uploads;
    }

    void Bind(const GDXDevice* device) const
    {
        if (!constantBuffer || !device) return;
        ID3D11DeviceContext* ctx = device->GetDeviceContext();
        if (!ctx) return;

        ctx->VSSetConstantBuffers(0, 1, &constantBuffer);
        ctx->PSSetConstantBuffers(0, 1, &constantBuffer);
        ++StatsStorage().binds;
    }

    ID3D11Buffer* constantBuffer = nullptr;

private:
    bool EnsureRingBuffers(const GDXDevice* device)
    {
        if (!device || !constantBuffer) return false;

        if (!m_constantBuffers[0])
            m_constantBuffers[0] = constantBuffer;

        if (m_ringInitialized)
            return true;

        ID3D11Device* dev = device->GetDevice();
        if (!dev) return false;

        D3D11_BUFFER_DESC desc{};
        constantBuffer->GetDesc(&desc);

        for (unsigned int i = 1; i < kBufferCount; ++i)
        {
            if (m_constantBuffers[i])
                continue;

            HRESULT hr = dev->CreateBuffer(&desc, nullptr, &m_constantBuffers[i]);
            if (FAILED(hr))
            {
                DBLOG_HR(hr);
                for (unsigned int j = 1; j < i; ++j)
                {
                    Memory::SafeRelease(m_constantBuffers[j]);
                }
                constantBuffer = m_constantBuffers[0];
                return true; // fallback auf Einzelbuffer statt kompletter Ausfall
            }
        }

        m_ringInitialized = true;
        return true;
    }

private:
    ID3D11Buffer* m_constantBuffers[kBufferCount] = {};
    unsigned int  m_currentBufferIndex = 0;
    bool          m_ringInitialized = false;

    static FrameStats& StatsStorage()
    {
        static FrameStats stats{};
        return stats;
    }
};
