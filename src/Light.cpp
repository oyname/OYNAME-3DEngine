// Light.cpp: No DX11. GPU upload goes through lightGpuData->Upload().
#include "Light.h"
#include "Dx11LightGpuData.h"

Light::Light() : Entity(EntityType::Light), lightType(LightType::Directional)
{
    DirectX::XMStoreFloat4(&cbLight.lightPosition,
        DirectX::XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });
    DirectX::XMStoreFloat4(&cbLight.lightDirection,
        DirectX::XMVECTOR{ 0.0f, 1.0f, 0.0f, 0.0f });
    SetDiffuseColor(DirectX::XMFLOAT4(1.0f, 0.8f, 1.0f, 100.0f));
    SetAmbientColor(DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
}

Light::~Light()
{
    delete lightGpuData;
    lightGpuData = nullptr;
}

static DirectX::XMVECTOR ChooseSafeUp(DirectX::XMVECTOR dir)
{
    using namespace DirectX;
    dir = XMVector3Normalize(dir);
    XMVECTOR upY = XMVectorSet(0, 1, 0, 0);
    float d = fabsf(XMVectorGetX(XMVector3Dot(dir, upY)));
    if (d > 0.98f)
        return XMVectorSet(0, 0, 1, 0);
    return upY;
}

DirectX::XMMATRIX Light::GetLightViewMatrix() const
{
    using namespace DirectX;
    XMVECTOR pos = XMLoadFloat4(&cbLight.lightPosition);
    pos = XMVectorSetW(pos, 1.0f);
    XMVECTOR dir = XMLoadFloat4(&cbLight.lightDirection);
    dir = XMVectorSetW(dir, 0.0f);
    dir = XMVector3Normalize(dir);
    if (XMVector3Equal(dir, XMVectorZero()))
        dir = XMVectorSet(0, -1, 0, 0);
    return XMMatrixLookToLH(pos, dir, ChooseSafeUp(dir));
}

DirectX::XMMATRIX Light::GetLightProjectionMatrix() const
{
    using namespace DirectX;
    if (lightType == LightType::Directional)
        return XMMatrixOrthographicLH(m_shadowOrthoSize, m_shadowOrthoSize, m_shadowNear, m_shadowFar);
    return XMMatrixPerspectiveFovLH(m_shadowFov, 1.0f, m_shadowNear, m_shadowFar);
}

void Light::SetShadowOrthoSize(float size)
{
    m_shadowOrthoSize = (size < 0.01f) ? 0.01f : size;
}

void Light::SetShadowPlanes(float nearPlane, float farPlane)
{
    if (nearPlane < 0.001f) nearPlane = 0.001f;
    if (farPlane <= nearPlane + 0.001f) farPlane = nearPlane + 0.001f;
    m_shadowNear = nearPlane;
    m_shadowFar = farPlane;
}

void Light::SetShadowFov(float fovRadians)
{
    const float minFov = DirectX::XMConvertToRadians(5.0f);
    const float maxFov = DirectX::XMConvertToRadians(170.0f);
    if (fovRadians < minFov) fovRadians = minFov;
    if (fovRadians > maxFov) fovRadians = maxFov;
    m_shadowFov = fovRadians;
}

void Light::Update(const GDXDevice* device)
{
    std::string key = "Light.cpp: ";
    DBLOG_ONCE(key.c_str(), "Update Light: ", Ptr(this).c_str());

    // Lights do not use the entity constant buffer - the renderer reads cbLight
    // (position, direction, color) via Dx11LightGpuData, never the entity world
    // matrix CB. Calling Entity::Update(device) would upload a useless CB and
    // increment EntityGpuData::FrameStats, inflating entityUploads/ringRotations.
    // Only the CPU-side world matrix needs refreshing so GetWorldMatrix() stays valid.
    if (m_active)
        matrixSet.worldMatrix = GetWorldMatrix();

    DirectX::XMVECTOR direction = transform.GetLookAt();
    DirectX::XMStoreFloat4(&cbLight.lightDirection, direction);

    DirectX::XMVECTOR pos = transform.GetPosition();
    DirectX::XMFLOAT4 posFloat;
    DirectX::XMStoreFloat4(&posFloat, pos);

    if (lightType == LightType::Directional)
        cbLight.lightPosition = DirectX::XMFLOAT4(posFloat.x, posFloat.y, posFloat.z, 0.0f);
    else if (lightType == LightType::Point)
        cbLight.lightPosition = DirectX::XMFLOAT4(posFloat.x, posFloat.y, posFloat.z, 1.0f);
}

void Light::SetDiffuseColor(const DirectX::XMFLOAT4& Color)
{
    float radius = cbLight.lightDiffuseColor.w;
    cbLight.lightDiffuseColor = DirectX::XMFLOAT4(Color.x, Color.y, Color.z, radius);
}

void Light::SetAmbientColor(const DirectX::XMFLOAT4& Color)
{
    cbLight.lightAmbientColor = Color;
}

void Light::SetLightType(LightType type)
{
    this->lightType = type;
}

void Light::SetLightType(D3DLIGHTTYPE d3dType)
{
    lightType = (d3dType == D3DLIGHT_POINT) ? LightType::Point : LightType::Directional;
}

void Light::SetRadius(float radius)
{
    cbLight.lightDiffuseColor.w = radius;
}

void Light::UpdateLight(const GDXDevice* device, DirectX::XMVECTOR position, DirectX::XMVECTOR lookAt)
{
    DirectX::XMFLOAT4 posFloat;
    DirectX::XMStoreFloat4(&posFloat, position);

    if (lightType == LightType::Directional)
        cbLight.lightPosition = DirectX::XMFLOAT4(posFloat.x, posFloat.y, posFloat.z, 0.0f);
    else
        cbLight.lightPosition = DirectX::XMFLOAT4(posFloat.x, posFloat.y, posFloat.z, 1.0f);

    DirectX::XMStoreFloat4(&cbLight.lightDirection, lookAt);

    if (lightGpuData) lightGpuData->Upload(device, cbLight);
}
