#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include "gdxutil.h"
#include "gdxdevice.h"


class Mesh;      // forward
class Material;  // forward

class Surface {
public:
    Surface();
    ~Surface();

    void AddVertex(unsigned int index, float x, float y, float z);
    void VertexNormal(unsigned int index, float x, float y, float z);
    void VertexColor(unsigned int index, float r, float g, float b);
    void VertexTexCoords(unsigned int index, float u, float v);
    void VertexTexCoords2(float u, float v);   // Lightmap / zweite UV
    void AddIndex(UINT index);

    void Draw(const GDXDevice* m_device, const DWORD flags);

    // Getter
    float GetVertexX(unsigned int index) const;
    float GetVertexY(unsigned int index) const;
    float GetVertexZ(unsigned int index) const;

    void CalculateSize(DirectX::XMMATRIX roationMatrix, DirectX::XMFLOAT3& minSize, DirectX::XMFLOAT3& maxSize);

public:
    bool isActive = false;

    std::vector<DirectX::XMFLOAT3> position;
    unsigned int size_position;
    unsigned int size_listPosition;

    std::vector<DirectX::XMFLOAT3> normal;
    unsigned int size_normal;
    unsigned int size_listNormal;

    std::vector<DirectX::XMFLOAT4> color;
    unsigned int size_color;
    unsigned int size_listColor;

    std::vector<DirectX::XMFLOAT2> uv1;
    unsigned int size_uv1;
    unsigned int size_listUV1;

    std::vector<DirectX::XMFLOAT2> uv2;
    unsigned int size_uv2;
    unsigned int size_listUV2;

    std::vector<unsigned int> indices;
    unsigned int size_listIndex;

    ID3D11Buffer* positionBuffer;
    ID3D11Buffer* normalBuffer;
    ID3D11Buffer* colorBuffer;
    ID3D11Buffer* uv1Buffer;
    ID3D11Buffer* uv2Buffer;
    ID3D11Buffer* indexBuffer;

    Mesh* pMesh = nullptr;
    Material* pMaterial = nullptr;   // pro-Surface Material (überschreibt mesh->pMaterial)
    DirectX::XMFLOAT3 minPoint{};
    DirectX::XMFLOAT3 maxPoint{};

public:
    // Debug Kollision...kann dann gelöscht werden!
    bool test;
};

typedef Surface* LPSURFACE;
typedef Surface SURFACE;