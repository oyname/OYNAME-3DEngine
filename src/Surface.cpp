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


// ---------------------------------------------------------------------------
// Tangent-Berechnung (Tangent-Space Normalmapping)
// Speichert float4(T.xyz, handedness) pro Vertex.
// Handedness = +1/-1, damit Bitangent im Shader rekonstruiert werden kann.
// ---------------------------------------------------------------------------
void Surface::ComputeTangents()
{
    const size_t vcount = m_positions.size();
    const size_t icount = m_indices.size();

    m_tangents.clear();
    m_tangents.resize(vcount, XMFLOAT4(1, 0, 0, 1));

    if (vcount == 0 || icount < 3)
        return;

    // Benötigt UV0 + Normalen
    if (m_uv1.size() != vcount || m_normals.size() != vcount)
        return;

    std::vector<XMFLOAT3> tan1(vcount, XMFLOAT3(0, 0, 0));
    std::vector<XMFLOAT3> tan2(vcount, XMFLOAT3(0, 0, 0));

    auto add3 = [](XMFLOAT3& a, const XMFLOAT3& b) { a.x += b.x; a.y += b.y; a.z += b.z; };

    for (size_t i = 0; i + 2 < icount; i += 3)
    {
        const uint32_t i0 = m_indices[i + 0];
        const uint32_t i1 = m_indices[i + 1];
        const uint32_t i2 = m_indices[i + 2];

        if (i0 >= vcount || i1 >= vcount || i2 >= vcount)
            continue;

        const XMFLOAT3& p0 = m_positions[i0];
        const XMFLOAT3& p1 = m_positions[i1];
        const XMFLOAT3& p2 = m_positions[i2];

        const XMFLOAT2& w0 = m_uv1[i0];
        const XMFLOAT2& w1 = m_uv1[i1];
        const XMFLOAT2& w2 = m_uv1[i2];

        const float x1 = p1.x - p0.x;
        const float y1 = p1.y - p0.y;
        const float z1 = p1.z - p0.z;

        const float x2 = p2.x - p0.x;
        const float y2 = p2.y - p0.y;
        const float z2 = p2.z - p0.z;

        const float s1 = w1.x - w0.x;
        const float t1u = w1.y - w0.y;

        const float s2 = w2.x - w0.x;
        const float t2u = w2.y - w0.y;

        const float det = (s1 * t2u - s2 * t1u);

        if (fabsf(det) < 1e-8f)
            continue;

        const float r = 1.0f / det;

        XMFLOAT3 sdir(
            (t2u * x1 - t1u * x2) * r,
            (t2u * y1 - t1u * y2) * r,
            (t2u * z1 - t1u * z2) * r
        );

        XMFLOAT3 tdir(
            (s1 * x2 - s2 * x1) * r,
            (s1 * y2 - s2 * y1) * r,
            (s1 * z2 - s2 * z1) * r
        );

        add3(tan1[i0], sdir); add3(tan1[i1], sdir); add3(tan1[i2], sdir);
        add3(tan2[i0], tdir); add3(tan2[i1], tdir); add3(tan2[i2], tdir);
    }

    for (size_t a = 0; a < vcount; ++a)
    {
        XMVECTOR n = XMLoadFloat3(&m_normals[a]);
        XMVECTOR t = XMLoadFloat3(&tan1[a]);
        XMVECTOR b = XMLoadFloat3(&tan2[a]);

        // Orthonormalize: t = normalize( t - n * dot(n,t) )
        t = XMVectorSubtract(t, XMVectorMultiply(n, XMVectorReplicate(XMVectorGetX(XMVector3Dot(n, t)))));

        // Falls degenerate, erzeuge irgendeinen Tangentenvektor
        const float tLenSq = XMVectorGetX(XMVector3LengthSq(t));
        if (tLenSq < 1e-10f)
        {
            // choose axis least parallel to normal
            XMVECTOR axis = (fabsf(XMVectorGetX(n)) < 0.9f) ? XMVectorSet(1,0,0,0) : XMVectorSet(0,1,0,0);
            t = XMVector3Normalize(XMVector3Cross(axis, n));
            b = XMVector3Cross(n, t);
        }
        else
        {
            t = XMVector3Normalize(t);
        }

        // handedness
        XMVECTOR c = XMVector3Cross(n, t);
        float handed = (XMVectorGetX(XMVector3Dot(c, b)) < 0.0f) ? -1.0f : 1.0f;

        XMFLOAT3 tf3;
        XMStoreFloat3(&tf3, t);
        m_tangents[a] = XMFLOAT4(tf3.x, tf3.y, tf3.z, handed);
    }
}
