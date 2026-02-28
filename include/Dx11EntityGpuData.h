#pragma once

// Vorwaertsdeklarationen - kein <d3d11.h> im Header.
struct ID3D11Buffer;
class  GDXDevice;
struct MatrixSet;

// EntityGpuData haelt den Matrix-Constant-Buffer einer Entity
// und die Map/Unmap-Logik fuer den GPU-Upload.
// Entity selbst kennt ID3D11Buffer nicht mehr.
class EntityGpuData
{
public:
    EntityGpuData();
    ~EntityGpuData();

    // Schreibt MatrixSet in den Constant Buffer und bindet ihn an b0.
    void Upload(const GDXDevice* device, const MatrixSet& matrices);

    ID3D11Buffer* constantBuffer = nullptr;
};
