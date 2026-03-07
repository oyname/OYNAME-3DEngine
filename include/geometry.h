#pragma once

#include "gidx.h"

void CreateCube(LPENTITY* mesh, MATERIAL* material = nullptr);

void CreateMultiMaterialCube(LPENTITY* mesh,
    LPMATERIAL matFront,    // Vorne/Hinten
    LPMATERIAL matSide,     // Links/Rechts
    LPMATERIAL matTopBot);  // Oben/Unten

void CreatePlate(LPENTITY* mesh);

// UV-Sphere als inline-Implementierung -- kein Eintrag in geometry.cpp noetig.
// rings = horizontale Ringe, segments = vertikale Segmente, radius in Welteinheiten.
#include <cmath>
inline void CreateSphere(LPENTITY* mesh, LPMATERIAL material = nullptr,
    int rings = 12, int segments = 16, float radius = 1.0f)
{
    Engine::CreateMesh(mesh);

    LPSURFACE surface = nullptr;
    Engine::CreateSurface(&surface, *mesh);
    if (material)
        Engine::SurfaceMaterial(surface, material);

    for (int r = 0; r <= rings; ++r)
    {
        const float phi = DirectX::XM_PI * static_cast<float>(r) / static_cast<float>(rings);
        const float sinPhi = std::sin(phi);
        const float cosPhi = std::cos(phi);

        for (int s = 0; s <= segments; ++s)
        {
            const float theta = DirectX::XM_2PI * static_cast<float>(s) / static_cast<float>(segments);
            const float sinTheta = std::sin(theta);
            const float cosTheta = std::cos(theta);

            const float x = radius * sinPhi * cosTheta;
            const float y = radius * cosPhi;
            const float z = radius * sinPhi * sinTheta;

            const float len = std::sqrt(x * x + y * y + z * z);
            const float nx = (len > 0.0f) ? x / len : 0.0f;
            const float ny = (len > 0.0f) ? y / len : 1.0f;
            const float nz = (len > 0.0f) ? z / len : 0.0f;

            Engine::AddVertex(surface, x, y, z);
            Engine::VertexNormal(surface, nx, ny, nz);
            Engine::VertexColor(surface, 255, 255, 255);
            Engine::VertexTexCoord(surface,
                static_cast<float>(s) / static_cast<float>(segments),
                static_cast<float>(r) / static_cast<float>(rings));
        }
    }

    const int stride = segments + 1;
    for (int r = 0; r < rings; ++r)
    {
        for (int s = 0; s < segments; ++s)
        {
            const int i0 = r * stride + s;
            const int i1 = r * stride + s + 1;
            const int i2 = (r + 1) * stride + s;
            const int i3 = (r + 1) * stride + s + 1;

            Engine::AddTriangle(surface, i0, i2, i1);
            Engine::AddTriangle(surface, i1, i2, i3);
        }
    }

    Engine::FillBuffer(surface);
}
