#pragma once
#include <d3d11.h>
#include "gdxutil.h"
#include "IGpuResource.h"

class GDXDevice; // forward

// DirectX-11-Implementierung von IGpuResource.
// Verwaltet die GPU-seitigen Vertex- und Index-Buffer einer Surface
// sowie den DX11-spezifischen Draw-Aufruf.
//
// Ausserhalb von gidx.h (FillBuffer/UpdateBuffer) sollte niemand
// direkt auf die Buffer-Member zugreifen -- nur IGpuResource-Interface nutzen.
class SurfaceGpuBuffer : public IGpuResource
{
public:
    ~SurfaceGpuBuffer() override;

    // IGpuResource
    void Draw   (const GDXDevice* device, unsigned int flagsVertex) const override;
    void Release()                                                         override;
    void SetWireframe(bool enabled) noexcept override { m_wireframe = enabled; }
    bool IsWireframe()        const noexcept override { return m_wireframe;    }

    // DX11-interne Buffer-Member -- nur fuer gidx.h::FillBuffer / UpdateBuffer
    ID3D11Buffer* positionBuffer = nullptr;
    ID3D11Buffer* normalBuffer   = nullptr;
    ID3D11Buffer* colorBuffer    = nullptr;
    ID3D11Buffer* uv1Buffer      = nullptr;
    ID3D11Buffer* uv2Buffer      = nullptr;
    ID3D11Buffer* indexBuffer    = nullptr;

    unsigned int stridePosition = 0;
    unsigned int strideNormal   = 0;
    unsigned int strideColor    = 0;
    unsigned int strideUV1      = 0;
    unsigned int strideUV2      = 0;

    unsigned int indexCount = 0;

private:
    bool m_wireframe = false;
};
