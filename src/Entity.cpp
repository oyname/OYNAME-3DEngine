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

    matrixSet.worldMatrix      = XMMatrixIdentity();
    matrixSet.viewMatrix       = XMMatrixIdentity();
    matrixSet.projectionMatrix = XMMatrixIdentity();

    viewport = {};
}

Entity::~Entity()
{
    delete gpuData;
    gpuData = nullptr;
}

void Entity::Update(const GDXDevice* device)
{
    if (!m_active) return;

    matrixSet.worldMatrix = transform.GetScaling() *
        transform.GetRotation() *
        transform.GetTranslation();

    if (gpuData) gpuData->Upload(device, matrixSet);
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
    viewport.x        = TopLeftX;
    viewport.y        = TopLeftY;
    viewport.width    = Width;
    viewport.height   = Height;
    viewport.minDepth = MinDepth;
    viewport.maxDepth = MaxDepth;
}
