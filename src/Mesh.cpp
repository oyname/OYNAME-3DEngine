#include "Memory.h"
#include "Mesh.h"
#include "Surface.h"
#include "GeometryHelper.h"
#include "gdxdevice.h"
using namespace DirectX;

Mesh::Mesh() :
    Entity(EntityType::Mesh),
    pMaterial(nullptr)
{
}

Mesh::~Mesh() {
    // Nur die Referenzen clearen, nicht die Surfaces löschen!
    m_surfaces.clear();
}

void Mesh::Update(const GDXDevice* device)
{
    Entity::Update(device);

    if (collisionType != COLLISION::NONE) {
        CalculateOBB(0);
    }
}

void Mesh::Update(const GDXDevice* device, const MatrixSet* inMatrixSet)
{
    if (!isActive) return;
    if (!device || !inMatrixSet) return;

    if (collisionType != COLLISION::NONE) {
        CalculateOBB(0);
    }

    MatrixSet ms = *inMatrixSet;
    ms.worldMatrix = transform.GetLocalTransformationMatrix();

    if (constantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        HRESULT hr = device->GetDeviceContext()->Map(
            constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

        if (FAILED(hr)) {
            Debug::LogHr(__FILE__, __LINE__, hr);
            return;
        }

        memcpy(mapped.pData, &ms, sizeof(MatrixSet));
        device->GetDeviceContext()->Unmap(constantBuffer, 0);

        device->GetDeviceContext()->VSSetConstantBuffers(0, 1, &constantBuffer);
        device->GetDeviceContext()->PSSetConstantBuffers(0, 1, &constantBuffer);
    }
}

Surface* Mesh::GetSurface(unsigned int n)
{
    if (n < m_surfaces.size()) {
        return m_surfaces[n];
    }
    return nullptr;
}

void Mesh::AddSurface(Surface* surface)
{
    if (!surface) return;
    m_surfaces.push_back(surface);
    surface->SetOwner(this);
}

void Mesh::RemoveSurface(Surface* surface)
{
    auto it = std::find(m_surfaces.begin(), m_surfaces.end(), surface);
    if (it != m_surfaces.end())
    {
        if (*it) (*it)->SetOwner(nullptr);
        m_surfaces.erase(it);
    }
}

void Mesh::SetCollisionMode(COLLISION collision)
{
    collisionType = collision;
}

void Mesh::CalculateOBB(unsigned int index)
{
    XMFLOAT3 minSize{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 maxSize{ 0.0f, 0.0f, 0.0f };

    if (m_surfaces.empty()) return;

    GeometryHelper::CalculateSize(*m_surfaces[0], XMMatrixIdentity(), minSize, maxSize);

    XMFLOAT3 extents{
        (maxSize.x - minSize.x) / 2.0f,
        (maxSize.y - minSize.y) / 2.0f,
        (maxSize.z - minSize.z) / 2.0f
    };

    XMMATRIX scalingMatrix = transform.GetScaling();
    float scaleX = XMVectorGetX(scalingMatrix.r[0]);
    float scaleY = XMVectorGetY(scalingMatrix.r[1]);
    float scaleZ = XMVectorGetZ(scalingMatrix.r[2]);

    extents.x *= scaleX;
    extents.y *= scaleY;
    extents.z *= scaleZ;

    XMFLOAT3 pos;
    XMStoreFloat3(&pos, transform.GetPosition());
    XMVECTOR quat = XMQuaternionRotationMatrix(transform.GetRotation());
    XMFLOAT4 quatFloat;
    XMStoreFloat4(&quatFloat, quat);

    obb = BoundingOrientedBox(pos, extents,
        XMFLOAT4(quatFloat.x, quatFloat.y, quatFloat.z, quatFloat.w)
    );
}

bool Mesh::CheckCollision(Mesh* mesh)
{
    if (collisionType == COLLISION::NONE || mesh->collisionType == COLLISION::NONE) {
        return false;
    }
    return obb.Intersects(mesh->obb);
}