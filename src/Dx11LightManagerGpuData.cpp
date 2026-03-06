// Dx11LightManagerGpuData.cpp: All DX11 calls for the light array buffer.
#include <d3d11.h>
#include "Dx11LightManagerGpuData.h"
//#include "LightManager.h"
#include "gdxdevice.h"
#include "gdxutil.h"

Dx11LightManagerGpuData::Dx11LightManagerGpuData()
{
}

Dx11LightManagerGpuData::~Dx11LightManagerGpuData()
{
    Memory::SafeRelease(m_lightBuffer);
}

bool Dx11LightManagerGpuData::Init(const GDXDevice* device, unsigned int bufferSizeBytes)
{
    if (!device)
        return false;

    Memory::SafeRelease(m_lightBuffer);

    D3D11_BUFFER_DESC desc{};
    desc.Usage          = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth      = bufferSizeBytes;
    desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device->GetDevice()->CreateBuffer(&desc, nullptr, &m_lightBuffer);
    if (FAILED(hr))
    {
        DBLOG_HR(hr);
        return false;
    }

    DBLOG("Dx11LightManagerGpuData.cpp: Light array buffer created (", bufferSizeBytes, " bytes)");
    return true;
}

void Dx11LightManagerGpuData::Upload(const GDXDevice* device, const LightArrayBuffer& data)
{
    if (!m_lightBuffer || !device)
        return;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = device->GetDeviceContext()->Map(
        m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    if (FAILED(hr))
    {
        DBLOG_HR(hr);
        return;
    }

    memcpy(mapped.pData, &data, sizeof(LightArrayBuffer));
    device->GetDeviceContext()->Unmap(m_lightBuffer, 0);

    device->GetDeviceContext()->VSSetConstantBuffers(1, 1, &m_lightBuffer);
    device->GetDeviceContext()->PSSetConstantBuffers(1, 1, &m_lightBuffer);
}
