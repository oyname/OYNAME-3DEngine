// Entity.cpp: No DX11. GPU upload goes through gpuData->Upload().
#include "Entity.h"
#include "Dx11EntityGpuData.h"

using namespace DirectX;

Entity::Entity(EntityType type)
    : m_entityType(type),
    m_active(true),
    m_visible(true),
    m_layerMask(LAYER_DEFAULT)
{
    transform.SetWorldMatrix(&matrixSet.worldMatrix);

    matrixSet.worldMatrix = XMMatrixIdentity();
    matrixSet.viewMatrix = XMMatrixIdentity();
    matrixSet.projectionMatrix = XMMatrixIdentity();

    viewport = {};
}

Entity::~Entity()
{
    // Cleanly detach from parent and notify all children
    DetachFromParent();
    for (Entity* child : m_children)
        if (child) child->m_parent = nullptr;
    m_children.clear();

    delete gpuData;
    gpuData = nullptr;
}

// Returns the full world matrix.
// If a parent exists, its world matrix is multiplied in recursively:
//   worldMatrix = localMatrix * parent->GetWorldMatrix()
// Without a parent this equals the local transformation matrix directly.
XMMATRIX Entity::GetWorldMatrix() const
{
    XMMATRIX local = transform.GetLocalTransformationMatrix();

    if (m_parent)
        return local * m_parent->GetWorldMatrix();

    return local;
}

void Entity::Update(const GDXDevice* device)
{
    if (!m_active) return;

    matrixSet.worldMatrix = GetWorldMatrix();

    if (gpuData) gpuData->Upload(device, matrixSet);
}

// Attaches this entity as a child of parent.
// Automatically registers it in the parent's children list.
// parent = nullptr is equivalent to DetachFromParent().
void Entity::SetParent(Entity* parent)
{
    if (parent == m_parent) return;

    // Zyklus-Guard: verhindert Stack Overflow durch rekursives GetWorldMatrix.
    // Traversiert die Parent-Chain des neuen Parents und prueft ob this darin vorkommt.
    if (parent)
    {
        Entity* p = parent;
        while (p)
        {
            if (p == this)
            {
                DBERROR("Entity.cpp: SetParent - cycle detected, assignment rejected");
                return;
            }
            p = p->m_parent;
        }
    }

    DetachFromParent();

    m_parent = parent;
    if (m_parent)
        m_parent->m_children.push_back(this);

    Debug::Log("Entity.cpp: SetParent - parent set");
}

void Entity::DetachFromParent()
{
    if (!m_parent) return;

    auto& siblings = m_parent->m_children;
    siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());

    m_parent = nullptr;
    Debug::Log("Entity.cpp: DetachFromParent - entity detached");
}

void Entity::GenerateViewMatrix(XMVECTOR position, XMVECTOR lookAt, XMVECTOR up)
{
    matrixSet.viewMatrix = XMMatrixLookAtLH(position, lookAt, up);
}

void Entity::GenerateProjectionMatrix(float fieldOfView, float screenAspect,
    float nearZ, float farZ)
{
    matrixSet.projectionMatrix =
        XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, nearZ, farZ);
}

void Entity::GenerateOrthoMatrix(float width, float height,
    float nearZ, float farZ)
{
    matrixSet.projectionMatrix =
        XMMatrixOrthographicLH(width, height, nearZ, farZ);
}

void Entity::GenerateViewport(float TopLeftX, float TopLeftY,
    float Width, float Height,
    float MinDepth, float MaxDepth)
{
    viewport.x = TopLeftX;
    viewport.y = TopLeftY;
    viewport.width = Width;
    viewport.height = Height;
    viewport.minDepth = MinDepth;
    viewport.maxDepth = MaxDepth;
}
