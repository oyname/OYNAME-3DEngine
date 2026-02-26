#pragma once
#include "IRenderTarget.h"

// Schritt 2: ShadowMapTarget ist nur noch ein "Tag"/Handle fuer das Shadow-Target.
// Alle DX11-Aufrufe (OMSetRenderTargets, Clear, SRV-Hazard, Viewport) laufen ueber das Backend.
class ShadowMapTarget : public IRenderTarget
{
public:
    // ShadowMap SRV wird vom Backend (DX11) aus dem GDXDevice geholt.
    // Damit bleibt dieses Interface hier API-neutral.
};
