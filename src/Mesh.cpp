#include "Memory.h"
#include "Mesh.h"
#include "GeometryHelper.h"
#include "gdxdevice.h"
using namespace DirectX;

Mesh::Mesh() :
    Entity(EntityType::Mesh),
    pMaterial(nullptr),
    collisionType(COLLISION::NONE)
{
}

Mesh::~Mesh() {
    // constantBuffer wird in Entity::~Entity() freigegeben
}

// ← Version 1: Einfaches Update (von Entity geerbt)
void Mesh::Update(const GDXDevice* device)
{
    // Basis-Update (Transform → Matrix)
    Entity::Update(device);

    // Mesh-spezifisch: Collision Box aktualisieren
    if (collisionType != COLLISION::NONE) {
        CalculateOBB(0);
    }
}

// ← Version 2: Rendering-Update mit Custom MatrixSet
void Mesh::Update(const GDXDevice* device, const MatrixSet* inMatrixSet)
{
    if (!isActive) return;
    if (!device || !inMatrixSet) return;

    // Collision Box aktualisieren
    if (collisionType != COLLISION::NONE) {
        CalculateOBB(0);
    }

    // Lokales Copy (verhindert, dass du externen Speicher “brauchst”)
    MatrixSet ms = *inMatrixSet;

    // World kommt IMMER aus dem Mesh-Transform
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


unsigned int Mesh::NumSurface()
{
    return this->surfaces.size();
}

Surface* Mesh::GetSurface(unsigned int n)
{
    if (n < this->surfaces.size()) {
        return this->surfaces[n];
    }
    return nullptr;
}

void Mesh::AddSurfaceToMesh(Surface* surface)
{
    this->surfaces.push_back(surface);
}

void Mesh::SetCollisionMode(COLLISION collision)
{
    collisionType = collision;
}

void Mesh::CalculateOBB(unsigned int index)
{
    using namespace DirectX;
    XMFLOAT3 minSize{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 maxSize{ 0.0f, 0.0f, 0.0f };

    if (this->surfaces.empty()) return;

    GeometryHelper::CalculateSize(*this->GetSurface(0), XMMatrixIdentity(), minSize, maxSize);

    // Center und Extents
    XMFLOAT3 extents{
        (maxSize.x - minSize.x) / 2.0f,
        (maxSize.y - minSize.y) / 2.0f,
        (maxSize.z - minSize.z) / 2.0f
    };

    // Extrahiere Skalierungsfaktoren
    XMMATRIX scalingMatrix = transform.GetScaling();
    float scaleX = XMVectorGetX(scalingMatrix.r[0]);
    float scaleY = XMVectorGetY(scalingMatrix.r[1]);
    float scaleZ = XMVectorGetZ(scalingMatrix.r[2]);

    // Wende Skalierung auf Extents an
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