#pragma once
#include <cstdint>
#include <DirectXMath.h>

// Vorwaertsdeklarationen - kein <d3d11.h> im Header.
struct ID3D11Buffer;
struct ID3D11DeviceContext;
class  GDXDevice;
struct LightBufferData;

// LightGpuData haelt alle DX11-Ressourcen eines Lichts:
// den Constant Buffer und die Map/Unmap-Logik fuer den GPU-Upload.
// Light selbst kennt diese Typen nicht mehr.
class LightGpuData
{
public:
    LightGpuData();
    ~LightGpuData();

    // Schreibt LightBufferData in den Constant Buffer und bindet ihn an b1.
    void Upload(const GDXDevice* device, const LightBufferData& data);

    ID3D11Buffer* lightBuffer = nullptr;
};
