#pragma once
#include <d3d11.h>

class RenderTarget
{
public:
    virtual ~RenderTarget() = default;

    // Rendertarget binden + internen State setzen (Viewport, RasterState)
    virtual void Bind() = 0;

    // Depth / Color-Buffer leeren
    virtual void Clear() = 0;

    // SRV-Slot freigeben bevor der Target als DSV genutzt wird (SRV-Hazard-Schutz)
    // Default-Implementierung: leer (nicht jeder Target braucht das)
    virtual void UnbindFromShader(unsigned int slot = 7) {}

    // Gibt den SRV zurück, falls vorhanden (z.B. ShadowMap für den Normalpass)
    // Default: nullptr (BackbufferTarget hat keinen SRV)
    virtual ID3D11ShaderResourceView* GetSRV() const { return nullptr; }
};
