#include "Entity.h"
#include "gdxdevice.h"
#include "Memory.h"
using namespace DirectX;

Entity::Entity()
    : constantBuffer(nullptr),
    m_active(true),
    m_visible(true),
    m_layerMask(LAYER_DEFAULT)
{
    transform.SetWorldMatrix(&matrixSet.worldMatrix);

    matrixSet.worldMatrix = DirectX::XMMatrixIdentity();
    matrixSet.viewMatrix = DirectX::XMMatrixIdentity();
    matrixSet.projectionMatrix = DirectX::XMMatrixIdentity();

    viewport = { 0 };
}

Entity::~Entity()
{
    Memory::SafeRelease(constantBuffer);
}

void Entity::Update(const GDXDevice* device)
{
    // active = false: komplett überspringen
    if (!m_active) return;

    HRESULT hr = S_OK;
    D3D11_MAPPED_SUBRESOURCE mappedResource;

    matrixSet.worldMatrix = transform.GetScaling() *
        transform.GetRotation() *
        transform.GetTranslation();

    if (constantBuffer != nullptr)
    {
        hr = device->GetDeviceContext()->Map(constantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(hr))
        {
            Debug::LogHr(__FILE__, __LINE__, hr);
            return;
        }

        memcpy(mappedResource.pData, &matrixSet, sizeof(MatrixSet));
        device->GetDeviceContext()->Unmap(constantBuffer, 0);
        device->GetDeviceContext()->VSSetConstantBuffers(0, 1, &constantBuffer);
        device->GetDeviceContext()->PSSetConstantBuffers(0, 1, &constantBuffer);
    }
}

void Entity::GenerateViewMatrix(DirectX::XMVECTOR position,
    DirectX::XMVECTOR lookAt,
    DirectX::XMVECTOR up)
{
    matrixSet.viewMatrix = DirectX::XMMatrixLookAtLH(position, lookAt, up);
}

void Entity::GenerateProjectionMatrix(float fieldOfView, float screenAspect,
    float nearZ, float farZ)
{
    matrixSet.projectionMatrix =
        DirectX::XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, nearZ, farZ);
}

void Entity::GenerateViewport(float TopLeftX, float TopLeftY,
    float Width, float Height,
    float MinDepth, float MaxDepth)
{
    viewport.Width = Width;
    viewport.Height = Height;
    viewport.MinDepth = MinDepth;
    viewport.MaxDepth = MaxDepth;
    viewport.TopLeftX = TopLeftX;
    viewport.TopLeftY = TopLeftY;
}