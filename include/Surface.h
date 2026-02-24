#pragma once
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "IGpuResource.h"

class Mesh;     // forward
class Material; // forward

// Surface beschreibt ausschliesslich die CPU-seitige Geometrie:
// Vertices (Position, Normal, Farbe, UV) und Indices.
//
// GPU-Ressourcen + Draw-Aufruf: gpu  (IGpuResource, konkret: SurfaceGpuBuffer)
// Geometrie-Berechnungen:       GeometryHelper::CalculateSize()
class Surface
{
public:
    Surface();
    ~Surface() = default;

    // Vertices hinzufuegen / setzen
    // index == -1 oder >= size: push_back; sonst: ueberschreiben
    void AddVertex       (int index, float x, float y, float z);
    void VertexNormal    (int index, float x, float y, float z);
    void VertexColor     (int index, float r, float g, float b);
    void VertexTexCoords (int index, float u, float v);
    void VertexTexCoords2(float u, float v);   // Lightmap / zweite UV

    void AddIndex(unsigned int index);

    // Getter - einzelne Vertex-Koordinaten
    float GetVertexX(unsigned int index) const;
    float GetVertexY(unsigned int index) const;
    float GetVertexZ(unsigned int index) const;

    // Getter - gesamte Datenreihen (const-Referenz, kein Kopieren)
    const std::vector<DirectX::XMFLOAT3>& GetPositions() const noexcept { return m_positions; }
    const std::vector<DirectX::XMFLOAT3>& GetNormals()   const noexcept { return m_normals;   }
    const std::vector<DirectX::XMFLOAT4>& GetColors()    const noexcept { return m_colors;    }
    const std::vector<DirectX::XMFLOAT2>& GetUV1()       const noexcept { return m_uv1;       }
    const std::vector<DirectX::XMFLOAT2>& GetUV2()       const noexcept { return m_uv2;       }
    const std::vector<unsigned int>&       GetIndices()   const noexcept { return m_indices;   }

    // Anzahl-Getter
    unsigned int CountVertices() const noexcept { return static_cast<unsigned int>(m_positions.size()); }
    unsigned int CountIndices()  const noexcept { return static_cast<unsigned int>(m_indices.size());   }
    unsigned int CountNormals()  const noexcept { return static_cast<unsigned int>(m_normals.size());   }
    unsigned int CountColors()   const noexcept { return static_cast<unsigned int>(m_colors.size());    }
    unsigned int CountUV1()      const noexcept { return static_cast<unsigned int>(m_uv1.size());       }
    unsigned int CountUV2()      const noexcept { return static_cast<unsigned int>(m_uv2.size());       }

public:
    // Navigation / Metadaten - nicht-owning Zeiger
    bool      isActive  = false;
    Mesh*     pMesh     = nullptr;
    Material* pMaterial = nullptr;  // pro-Surface Material (ueberschreibt mesh->pMaterial)

    // GPU Resource Layer - Interface, konkret DX11: SurfaceGpuBuffer
    // RenderManager spricht nur gegen IGpuResource.
    // gidx.h::FillBuffer castet auf SurfaceGpuBuffer* fuer Buffer-Upload.
    std::unique_ptr<IGpuResource> gpu;

private:
    // CPU-Geometriedaten
    std::vector<DirectX::XMFLOAT3> m_positions;
    std::vector<DirectX::XMFLOAT3> m_normals;
    std::vector<DirectX::XMFLOAT4> m_colors;
    std::vector<DirectX::XMFLOAT2> m_uv1;
    std::vector<DirectX::XMFLOAT2> m_uv2;
    std::vector<unsigned int>      m_indices;
};

typedef Surface* LPSURFACE;
typedef Surface  SURFACE;
