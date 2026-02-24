#include "Surface.h"
using namespace DirectX;

Surface::Surface() :
    size_position(0),
    size_listPosition(0),
    size_color(0),
    size_listColor(0),
    size_listIndex(0),
    size_normal(0),
    size_listNormal(0),
    positionBuffer(nullptr),
    colorBuffer(nullptr),
    indexBuffer(nullptr),
    normalBuffer(nullptr),
    uv1Buffer(nullptr),
    uv2Buffer(nullptr),
    size_listUV1(0),
    size_listUV2(0),
    size_uv1(0),
    size_uv2(0),
    pMesh(nullptr),
    isActive(true),
    m_wireframe(false)

{
}

Surface::~Surface() {
    isActive = false;
    Memory::SafeRelease(positionBuffer);
    Memory::SafeRelease(colorBuffer);
    Memory::SafeRelease(indexBuffer);
    Memory::SafeRelease(normalBuffer);
    Memory::SafeRelease(uv1Buffer);
    Memory::SafeRelease(uv2Buffer);
}

float Surface::GetVertexX(unsigned int index) const
{
    if (index >= position.size())
        return 0.0f;
    return position[index].x;
}

float Surface::GetVertexY(unsigned int index) const
{
    if (index >= position.size())
        return 0.0f;
    return position[index].y;
}

float Surface::GetVertexZ(unsigned int index) const
{
    if (index >= position.size())
        return 0.0f;
    return position[index].z;
}

void Surface::AddVertex(unsigned int index, float x, float y, float z)
{
    if (index < position.size()) {
        position[index] = DirectX::XMFLOAT3(x, y, z);
    }
    else {
        position.push_back(DirectX::XMFLOAT3(x, y, z));
    }
    size_listPosition = (unsigned int)position.size();
    size_position = sizeof(DirectX::XMFLOAT3);
}

void Surface::VertexColor(unsigned int index, float r, float g, float b)
{
    if (index < color.size()) {
        color[index] = DirectX::XMFLOAT4(r, g, b, 1.0f);
    }
    else {
        color.push_back(DirectX::XMFLOAT4(r, g, b, 1.0f));
    }
    size_listColor = static_cast<unsigned int>(color.size());
    size_color = sizeof(DirectX::XMFLOAT4);
}

void Surface::VertexNormal(unsigned int index, float x, float y, float z)
{
    if (index < normal.size()) {
        normal[index] = DirectX::XMFLOAT3(x, y, z);
    }
    else {
        normal.push_back(DirectX::XMFLOAT3(x, y, z));
    }
    size_listNormal = (unsigned int)normal.size();
    size_normal = sizeof(DirectX::XMFLOAT3);
}

void Surface::VertexTexCoords(unsigned int index, float u, float v)
{
    if (index < uv1.size()) {               // ← FIXED: removed >= 0 check
        uv1[index] = DirectX::XMFLOAT2(u, v);
    }
    else {
        uv1.push_back(DirectX::XMFLOAT2(u, v));
    }
    size_listUV1 = (unsigned int)uv1.size();
    size_uv1 = sizeof(DirectX::XMFLOAT2);
}

void Surface::VertexTexCoords2(float u, float v)
{
    uv2.push_back(DirectX::XMFLOAT2(u, v));
    size_listUV2 = (unsigned int)uv2.size();
    size_uv2 = sizeof(DirectX::XMFLOAT2);
}

void Surface::AddIndex(UINT index)
{
    indices.push_back(index);
    size_listIndex = (unsigned int)indices.size();
}

void Surface::Draw(const GDXDevice* device, const DWORD flagsVertex)
{
    // Alle aktiven Streams in einem einzigen IASetVertexBuffers-Call binden
    ID3D11Buffer* buffers[5] = {};
    UINT          strides[5] = {};
    UINT          offsets[5] = {};
    UINT          cnt = 0;

    if (flagsVertex & D3DVERTEX_POSITION) { buffers[cnt] = positionBuffer; strides[cnt] = size_position; cnt++; }
    if (flagsVertex & D3DVERTEX_NORMAL) { buffers[cnt] = normalBuffer;   strides[cnt] = size_normal;   cnt++; }
    if (flagsVertex & D3DVERTEX_COLOR) { buffers[cnt] = colorBuffer;    strides[cnt] = size_color;    cnt++; }
    if (flagsVertex & D3DVERTEX_TEX1) { buffers[cnt] = uv1Buffer;      strides[cnt] = size_uv1;      cnt++; }
    if (flagsVertex & D3DVERTEX_TEX2) { buffers[cnt] = uv2Buffer;      strides[cnt] = size_uv2;      cnt++; }

    if (cnt > 0)
        device->GetDeviceContext()->IASetVertexBuffers(0, cnt, buffers, strides, offsets);

    device->GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (!m_wireframe)
    {
        device->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device->GetDeviceContext()->DrawIndexed(size_listIndex, 0, 0);
    }
    else
    {
        device->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        device->GetDeviceContext()->Draw(size_listIndex, 0);
    }
}

void Surface::CalculateSize(XMMATRIX rotationMatrix, XMFLOAT3& minSize, XMFLOAT3& maxSize)
{
    minPoint.x = FLT_MAX;
    minPoint.y = FLT_MAX;
    minPoint.z = FLT_MAX;
    maxPoint.x = -FLT_MAX;
    maxPoint.y = -FLT_MAX;
    maxPoint.z = -FLT_MAX;

    for (const auto& vertex : position)
    {
        XMVECTOR rotatedVertex = XMVector3Transform(XMLoadFloat3(&vertex), rotationMatrix);
        XMFLOAT3 transformedCoords;
        XMStoreFloat3(&transformedCoords, rotatedVertex);

        if (transformedCoords.x < minPoint.x)
            minPoint.x = transformedCoords.x;
        if (transformedCoords.y < minPoint.y)
            minPoint.y = transformedCoords.y;
        if (transformedCoords.z < minPoint.z)
            minPoint.z = transformedCoords.z;

        if (transformedCoords.x > maxPoint.x)
            maxPoint.x = transformedCoords.x;
        if (transformedCoords.y > maxPoint.y)
            maxPoint.y = transformedCoords.y;
        if (transformedCoords.z > maxPoint.z)
            maxPoint.z = transformedCoords.z;
    }

    minSize = minPoint;
    maxSize = maxPoint;
}
