#pragma once
#include <DirectXMath.h>
#include "Light.h"

#define MAX_LIGHTS 32

// CPU-seitiger Array-Buffer fuer alle Lichter.
// Kein DX11, nur Daten.
struct LightArrayBuffer
{
    LightBufferData   lights[MAX_LIGHTS];
    unsigned int      lightCount;
    DirectX::XMFLOAT3 padding;
};
