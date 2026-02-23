#pragma once
#include "RenderTarget.h"
#include <d3d11.h>

class GDXDevice;

class ShadowMapTarget : public RenderTarget
{
public:
    void SetDevice(GDXDevice* device) { m_device = device; }

    // IRenderTarget
    void Bind()                               override;
    void Clear()                              override;
    void UnbindFromShader(unsigned int slot = 7) override;
    ID3D11ShaderResourceView* GetSRV() const  override;

private:
    GDXDevice* m_device = nullptr; // non-owning
};
