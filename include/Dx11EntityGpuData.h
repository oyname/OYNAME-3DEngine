#pragma once

#include <d3d11.h>
#include <cstring>
#include "gdxutil.h"
#include "gdxdevice.h"

struct MatrixSet;

// EntityGpuData haelt den Matrix-Constant-Buffer einer Entity
// und die Map/Unmap-Logik fuer den GPU-Upload.
// Entity selbst kennt die Logik nur ueber diesen Typ.
class EntityGpuData
{
public:
    EntityGpuData() = default;

    ~EntityGpuData()
    {
        Memory::SafeRelease(constantBuffer);
    }

    // Schreibt MatrixSet in den Constant Buffer und bindet ihn an b0.
    void Upload(const GDXDevice* device, const MatrixSet& matrices)
    {
        if (!constantBuffer || !device) return;

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
        device->GetDeviceContext()->VSSetConstantBuffers(0, 1, &constantBuffer);
        device->GetDeviceContext()->PSSetConstantBuffers(0, 1, &constantBuffer);
    }

    ID3D11Buffer* constantBuffer = nullptr;
};
