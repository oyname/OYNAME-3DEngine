// Dx11EntityGpuData.cpp: alle DX11-Aufrufe fuer Entity-GPU-Ressourcen.
#include <d3d11.h>
#include "gdxutil.h"
#include "gdxdevice.h"
#include "Dx11EntityGpuData.h"

EntityGpuData::EntityGpuData()
{
}

EntityGpuData::~EntityGpuData()
{
    Memory::SafeRelease(constantBuffer);
}

void EntityGpuData::Upload(const GDXDevice* device, const MatrixSet& matrices)
{
    if (!constantBuffer || !device) return;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = device->GetDeviceContext()->Map(
        constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return;
    }

    memcpy(mapped.pData, &matrices, sizeof(MatrixSet));
    device->GetDeviceContext()->Unmap(constantBuffer, 0);
    device->GetDeviceContext()->VSSetConstantBuffers(0, 1, &constantBuffer);
    device->GetDeviceContext()->PSSetConstantBuffers(0, 1, &constantBuffer);
}
