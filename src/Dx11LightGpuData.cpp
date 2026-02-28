// Dx11LightGpuData.cpp: alle DX11-Aufrufe fuer Licht-GPU-Ressourcen.
#include <d3d11.h>
#include "gdxutil.h"
#include "gdxdevice.h"
#include "Light.h"
#include "Dx11LightGpuData.h"

LightGpuData::LightGpuData()
{
}

LightGpuData::~LightGpuData()
{
    Memory::SafeRelease(lightBuffer);
}

void LightGpuData::Upload(const GDXDevice* device, const LightBufferData& data)
{
    if (!lightBuffer || !device) return;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = device->GetDeviceContext()->Map(
        lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    if (FAILED(hr))
    {
        Debug::Log("Dx11LightGpuData.cpp: Upload - Map fehlgeschlagen");
        return;
    }

    memcpy(mapped.pData, &data, sizeof(LightBufferData));
    device->GetDeviceContext()->Unmap(lightBuffer, 0);

    device->GetDeviceContext()->VSSetConstantBuffers(1, 1, &lightBuffer);
    device->GetDeviceContext()->PSSetConstantBuffers(1, 1, &lightBuffer);
}
