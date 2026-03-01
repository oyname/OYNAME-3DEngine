// Entity.cpp: kein DX11. GPU-Upload laeuft ueber gpuData->Upload().
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
    // Sauber vom Parent trennen und alle Kinder benachrichtigen
    DetachFromParent();
    for (Entity* child : m_children)
        if (child) child->m_parent = nullptr;
    m_children.clear();

    delete gpuData;
    gpuData = nullptr;
}

// Liefert die vollstaendige Weltmatrix.
// Wenn ein Parent vorhanden ist, wird dessen Weltmatrix rekursiv davorgemultipliziert:
//   worldMatrix = localMatrix * parent->GetWorldMatrix()
// Ohne Parent entspricht das direkt der lokalen Transformationsmatrix.
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

// Haengt diese Entity als Kind an parent.
// Traegt sie automatisch in die Children-Liste des Parents ein.
// parent = nullptr entspricht DetachFromParent().
void Entity::SetParent(Entity* parent)
{
    if (parent == m_parent) return;

    DetachFromParent();

    m_parent = parent;
    if (m_parent)
        m_parent->m_children.push_back(this);

    Debug::Log("Entity.cpp: SetParent - Parent gesetzt");
}

// Trennt diese Entity vom Parent.
void Entity::DetachFromParent()
{
    if (!m_parent) return;

    auto& siblings = m_parent->m_children;
    siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());

    m_parent = nullptr;
    Debug::Log("Entity.cpp: DetachFromParent - Entity abgetrennt");
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
