#pragma once
#include <DirectXMath.h>

class Surface; // forward

// Statische Geometrie-Hilfsfunktionen.
// Berechnung geometrischer Eigenschaften geh\xf6rt nicht in den Datenbeh\xe4lter Surface.
class GeometryHelper
{
public:
    GeometryHelper() = delete;

    // Berechnet AABB der Surface-Vertices nach optionaler Rotation.
    // Schreibt Ergebnis in minOut / maxOut.
    static void CalculateSize(
        const Surface&          surface,
        DirectX::XMMATRIX       rotationMatrix,
        DirectX::XMFLOAT3&      minOut,
        DirectX::XMFLOAT3&      maxOut);
};
