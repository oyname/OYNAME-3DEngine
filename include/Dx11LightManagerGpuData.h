#pragma once

// Vorwaertsdeklarationen - kein <d3d11.h> im Header.
struct ID3D11Buffer;
class  GDXDevice;
#include "LightArrayBuffer.h"

// Haelt alle DX11-Ressourcen des LightManagers:
// den Array-Constant-Buffer und die Map/Unmap-Logik fuer den GPU-Upload.
// LightManager selbst kennt ID3D11Buffer nicht mehr.
class Dx11LightManagerGpuData
{
public:
    Dx11LightManagerGpuData();
    ~Dx11LightManagerGpuData();

    // Buffer anlegen (einmalig oder bei Groessenaenderung).
    bool Init(const GDXDevice* device, unsigned int bufferSizeBytes);

    // LightArrayBuffer in den Constant Buffer schreiben und an b1 binden.
    void Upload(const GDXDevice* device, const LightArrayBuffer& data);

    bool IsReady() const { return m_lightBuffer != nullptr; }

private:
    ID3D11Buffer* m_lightBuffer = nullptr;
};
