#pragma once
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "IGpuResource.h"

class Mesh;     // forward
class Material; // forward

// Surface besitzt die CPU-Geometrie und die GPU-Ressource
// Es hat einen nicht-owning Pointer zu seinem Besitzer-Mesh
class Surface
{
public:
    Surface();
    ~Surface() = default;

    // Besitzer setzen (nur durch ObjectManager)
    void SetOwner(Mesh* mesh) { m_owner = mesh; pMesh = mesh; }
    Mesh* GetOwner() const { return m_owner; }

    // Vertices hinzufuegen / setzen
    void AddVertex(int index, float x, float y, float z);
    void VertexNormal(int index, float x, float y, float z);
    void VertexColor(int index, float r, float g, float b);
    void VertexTexCoords(int index, float u, float v);
    void VertexTexCoords2(float u, float v);

    void AddIndex(unsigned int index);

    // Getter
    float GetVertexX(unsigned int index) const;
    float GetVertexY(unsigned int index) const;
    float GetVertexZ(unsigned int index) const;

    const std::vector<DirectX::XMFLOAT3>& GetPositions() const noexcept { return m_positions; }
    const std::vector<DirectX::XMFLOAT3>& GetNormals()   const noexcept { return m_normals; }
    const std::vector<DirectX::XMFLOAT4>& GetColors()    const noexcept { return m_colors; }
    const std::vector<DirectX::XMFLOAT2>& GetUV1()       const noexcept { return m_uv1; }
    const std::vector<DirectX::XMFLOAT2>& GetUV2()       const noexcept { return m_uv2; }
    const std::vector<DirectX::XMFLOAT4>& GetTangents()  const noexcept { return m_tangents; }
    const std::vector<unsigned int>& GetIndices()   const noexcept { return m_indices; }

    unsigned int CountVertices() const noexcept { return static_cast<unsigned int>(m_positions.size()); }
    unsigned int CountIndices()  const noexcept { return static_cast<unsigned int>(m_indices.size()); }
    unsigned int CountNormals()  const noexcept { return static_cast<unsigned int>(m_normals.size()); }
    unsigned int CountColors()   const noexcept { return static_cast<unsigned int>(m_colors.size()); }
    unsigned int CountUV1()      const noexcept { return static_cast<unsigned int>(m_uv1.size()); }
    unsigned int CountUV2()      const noexcept { return static_cast<unsigned int>(m_uv2.size()); }
    unsigned int CountTangents() const noexcept { return static_cast<unsigned int>(m_tangents.size()); }

    // Berechnet Tangenten (xyz) + Handedness (w) pro Vertex aus Position + UV0 + Indices
    // Voraussetzung: m_positions, m_normals, m_uv1, m_indices sind gefuellt.
    void ComputeTangents();

public:
    bool      isActive = true;
    Mesh* pMesh = nullptr;     // non-owning, gleiche wie m_owner
    Material* pMaterial = nullptr;      // non-owning, kann mesh->pMaterial ueberschreiben

    std::unique_ptr<IGpuResource> gpu;  // owned

private:
    std::vector<DirectX::XMFLOAT3> m_positions;
    std::vector<DirectX::XMFLOAT3> m_normals;
    std::vector<DirectX::XMFLOAT4> m_colors;
    std::vector<DirectX::XMFLOAT2> m_uv1;
    std::vector<DirectX::XMFLOAT2> m_uv2;
    std::vector<DirectX::XMFLOAT4> m_tangents;
    std::vector<unsigned int>      m_indices;
    Mesh* m_owner = nullptr;            // non-owning
};

typedef Surface* LPSURFACE;
typedef Surface  SURFACE;
