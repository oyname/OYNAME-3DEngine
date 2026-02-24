#include "Surface.h"
#include "SurfaceGpuBuffer.h"

using namespace DirectX;

Surface::Surface()
    : isActive(true)
    , pMesh(nullptr)
    , pMaterial(nullptr)
    , gpu(std::make_unique<SurfaceGpuBuffer>())
{
}

float Surface::GetVertexX(unsigned int index) const
{
    return (index < m_positions.size()) ? m_positions[index].x : 0.0f;
}

float Surface::GetVertexY(unsigned int index) const
{
    return (index < m_positions.size()) ? m_positions[index].y : 0.0f;
}

float Surface::GetVertexZ(unsigned int index) const
{
    return (index < m_positions.size()) ? m_positions[index].z : 0.0f;
}

void Surface::AddVertex(int index, float x, float y, float z)
{
    if (index >= 0 && static_cast<unsigned int>(index) < m_positions.size())
        m_positions[index] = XMFLOAT3(x, y, z);
    else
        m_positions.push_back(XMFLOAT3(x, y, z));
}

void Surface::VertexColor(int index, float r, float g, float b)
{
    if (index >= 0 && static_cast<unsigned int>(index) < m_colors.size())
        m_colors[index] = XMFLOAT4(r, g, b, 1.0f);
    else
        m_colors.push_back(XMFLOAT4(r, g, b, 1.0f));
}

void Surface::VertexNormal(int index, float x, float y, float z)
{
    if (index >= 0 && static_cast<unsigned int>(index) < m_normals.size())
        m_normals[index] = XMFLOAT3(x, y, z);
    else
        m_normals.push_back(XMFLOAT3(x, y, z));
}

void Surface::VertexTexCoords(int index, float u, float v)
{
    if (index >= 0 && static_cast<unsigned int>(index) < m_uv1.size())
        m_uv1[index] = XMFLOAT2(u, v);
    else
        m_uv1.push_back(XMFLOAT2(u, v));
}

void Surface::VertexTexCoords2(float u, float v)
{
    m_uv2.push_back(XMFLOAT2(u, v));
}

void Surface::AddIndex(unsigned int index)
{
    m_indices.push_back(index);
}
