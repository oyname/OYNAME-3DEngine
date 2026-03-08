// Mesh.cpp: No direct DX11. GPU upload goes through gpuData->Upload().
#include "Mesh.h"
#include "Surface.h"
#include "MeshAsset.h"
#include "GeometryHelper.h"
#include "Dx11EntityGpuData.h"
using namespace DirectX;

Mesh::Mesh() :
    Entity(EntityType::Mesh)
{
}

Mesh::~Mesh()
{
    if (boneConstantBuffer)
    {
        boneConstantBuffer->Release();
        boneConstantBuffer = nullptr;
    }
}

void Mesh::Update(const GDXDevice* device)
{
    Entity::Update(device);

    if (collisionType != COLLISION::NONE)
        CalculateOBB(0);
}

void Mesh::Update(const GDXDevice* device, const MatrixSet* inMatrixSet)
{
    if (!isActive) return;
    if (!device || !inMatrixSet) return;

    if (collisionType != COLLISION::NONE)
        CalculateOBB(0);

    if (gpuData) gpuData->Upload(device, *inMatrixSet);
}

Surface* Mesh::GetSurface(unsigned int n)
{
    return m_meshRenderer.GetAsset() ? m_meshRenderer.GetAsset()->GetSlot(n) : nullptr;
}


bool Mesh::TryGetSurfaceSlot(const Surface* surface, unsigned int& outSlot) const noexcept
{
    const MeshAsset* asset = m_meshRenderer.GetAsset();
    return asset ? asset->FindSlotIndex(surface, outSlot) : false;
}

void Mesh::AddSurface(Surface* surface)
{
    if (!surface) return;
    MeshAsset* asset = AccessMeshAssetInternal();
    if (!asset) return;

    asset->AddSlot(surface);
}

void Mesh::RemoveSurface(Surface* surface)
{
    MeshAsset* asset = AccessMeshAssetInternal();
    if (!asset) return;

    asset->RemoveSlot(surface);
}

void Mesh::SetCollisionMode(COLLISION collision)
{
    collisionType = collision;
}

void Mesh::CalculateOBB(unsigned int index)
{
    XMFLOAT3 minSize{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 maxSize{ 0.0f, 0.0f, 0.0f };

    Surface* s0 = m_meshRenderer.GetAsset() ? m_meshRenderer.GetAsset()->GetSlot(0) : nullptr;
    if (!s0) return;

    GeometryHelper::CalculateSize(*s0, XMMatrixIdentity(), minSize, maxSize);

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
        XMFLOAT4(quatFloat.x, quatFloat.y, quatFloat.z, quatFloat.w));
}

bool Mesh::CheckCollision(Mesh* mesh)
{
    if (collisionType == COLLISION::NONE || mesh->collisionType == COLLISION::NONE)
        return false;
    return obb.Intersects(mesh->obb);
}
