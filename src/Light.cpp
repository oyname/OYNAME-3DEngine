#include "Light.h"
#include "Memory.h"

Light::Light() : Entity(EntityType::Light), lightType(LightType::Directional)
{
    lightBuffer = nullptr;

    // Initialisiere Position mit W=0 (direktionales Licht)
    DirectX::XMStoreFloat4(&cbLight.lightPosition,
        DirectX::XMVECTOR{ 0.0f, 0.0f, 0.0f, 0.0f });

    // Up-Vector
    DirectX::XMStoreFloat4(&cbLight.lightDirection,
        DirectX::XMVECTOR{ 0.0f, 1.0f, 0.0f, 0.0f });

    // Setze Diffuse mit Default-Radius
    SetDiffuseColor(DirectX::XMFLOAT4(1.0f, 0.8f, 1.0f, 100.0f));  // A = Radius

    // Setze Ambient
    SetAmbientColor(DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
}

Light::~Light()
{
    Memory::SafeRelease(lightBuffer);
}

// ==================== SHADOW MAPPING HELPERS ====================
// Robust "up" handling: if direction is almost parallel to (0,1,0), use (0,0,1).
static DirectX::XMVECTOR ChooseSafeUp(DirectX::XMVECTOR dir)
{
    using namespace DirectX;
    dir = XMVector3Normalize(dir);
    XMVECTOR upY = XMVectorSet(0, 1, 0, 0);
    float d = fabsf(XMVectorGetX(XMVector3Dot(dir, upY)));
    if (d > 0.98f) {
        return XMVectorSet(0, 0, 1, 0);
    }
    return upY;
}

DirectX::XMMATRIX Light::GetLightViewMatrix() const
{
    using namespace DirectX;

    // Position comes from cbLight (kept in Update())
    XMVECTOR pos = XMLoadFloat4(&cbLight.lightPosition);
    pos = XMVectorSetW(pos, 1.0f);

    // Direction comes from cbLight.lightDirection (set in Update/UpdateLight)
    XMVECTOR dir = XMLoadFloat4(&cbLight.lightDirection);
    dir = XMVectorSetW(dir, 0.0f);
    dir = XMVector3Normalize(dir);
    if (XMVector3Equal(dir, XMVectorZero())) {
        dir = XMVectorSet(0, -1, 0, 0);
    }

    XMVECTOR up = ChooseSafeUp(dir);

    // LookTo is the natural fit here (pos + direction)
    return XMMatrixLookToLH(pos, dir, up);
}

DirectX::XMMATRIX Light::GetLightProjectionMatrix() const
{
    using namespace DirectX;

    if (lightType == LightType::Directional) {
        // Square orthographic frustum: width=height=m_shadowOrthoSize
        return XMMatrixOrthographicLH(m_shadowOrthoSize, m_shadowOrthoSize, m_shadowNear, m_shadowFar);
    }

    // Point/Spot style: square aspect for shadow map
    return XMMatrixPerspectiveFovLH(m_shadowFov, 1.0f, m_shadowNear, m_shadowFar);
}

void Light::SetShadowOrthoSize(float size)
{
    m_shadowOrthoSize = (size < 0.01f) ? 0.01f : size;
}

void Light::SetShadowPlanes(float nearPlane, float farPlane)
{
    // Basic sanity; avoid invalid projection.
    if (nearPlane < 0.001f) nearPlane = 0.001f;
    if (farPlane <= nearPlane + 0.001f) farPlane = nearPlane + 0.001f;
    m_shadowNear = nearPlane;
    m_shadowFar = farPlane;
}

void Light::SetShadowFov(float fovRadians)
{
    // Clamp to a reasonable range: ~5° .. ~170°
    const float minFov = DirectX::XMConvertToRadians(5.0f);
    const float maxFov = DirectX::XMConvertToRadians(170.0f);
    if (fovRadians < minFov) fovRadians = minFov;
    if (fovRadians > maxFov) fovRadians = maxFov;
    m_shadowFov = fovRadians;
}

// WICHTIG: Update() überschreiben - berechnet lightDirection 
// automatisch aus der Transform-Rotation
void Light::Update(const GDXDevice* device)
{
    std::string key = "Light.cpp: ";
    Debug::LogOnce(key.c_str(), "Update Light: ", Ptr(this).c_str());

    // Basis-Update aufrufen (Matrix-Berechnungen)
    Entity::Update(device);

    // lightDirection aus der aktuellen Transform-Rotation berechnen
    // LookAt-Vektor ist die Forward-Richtung nach der Rotation
    DirectX::XMVECTOR direction = transform.GetLookAt();
    DirectX::XMStoreFloat4(&cbLight.lightDirection, direction);

    // Aktualisiere lightPosition basierend auf Licht-Typ
    DirectX::XMVECTOR pos = transform.GetPosition();

    if (lightType == LightType::Directional)
    {
        // Für direktionale Lichter: W=0 (keine Position-Berechnung nötig)
        DirectX::XMFLOAT4 posFloat;
        DirectX::XMStoreFloat4(&posFloat, pos);
        cbLight.lightPosition = DirectX::XMFLOAT4(posFloat.x, posFloat.y, posFloat.z, 0.0f);
    }
    else if (lightType == LightType::Point)
    {
        // Für Point-Lichter: W=1 (Position wird im Shader genutzt)
        DirectX::XMFLOAT4 posFloat;
        DirectX::XMStoreFloat4(&posFloat, pos);
        cbLight.lightPosition = DirectX::XMFLOAT4(posFloat.x, posFloat.y, posFloat.z, 1.0f);
    }


}

void Light::SetDiffuseColor(const DirectX::XMFLOAT4& Color)
{
    // Behalte den Radius (W-Komponente) wenn bereits gesetzt
    float radius = cbLight.lightDiffuseColor.w;
    cbLight.lightDiffuseColor = DirectX::XMFLOAT4(Color.x, Color.y, Color.z, radius);
}

void Light::SetAmbientColor(const DirectX::XMFLOAT4& Color)
{
    cbLight.lightAmbientColor = Color;
}

// Neue API: LightType
void Light::SetLightType(LightType type)
{
    this->lightType = type;
}

// Alte API: D3DLIGHTTYPE (Rückwärtskompatibilität)
void Light::SetLightType(D3DLIGHTTYPE d3dType)
{
    // Konvertiere D3DLIGHTTYPE zu LightType
    // D3DLIGHT_DIRECTIONAL = 1, D3DLIGHT_POINT = 2, D3DLIGHT_SPOT = 3
    if (d3dType == D3DLIGHT_POINT) {
        lightType = LightType::Point;
    }
    else {
        lightType = LightType::Directional;  // Default für alles andere
    }
}

void Light::SetRadius(float radius)
{
    // Aktualisiere nur die A-Komponente (Radius)
    cbLight.lightDiffuseColor.w = radius;
}

void Light::UpdateLight(const GDXDevice* device, DirectX::XMVECTOR position, DirectX::XMVECTOR lookAt)
{
    HRESULT hr = S_OK;

    // Aktualisiere Position basierend auf Licht-Typ
    DirectX::XMFLOAT4 posFloat;
    DirectX::XMStoreFloat4(&posFloat, position);

    if (lightType == LightType::Directional)
    {
        cbLight.lightPosition = DirectX::XMFLOAT4(posFloat.x, posFloat.y, posFloat.z, 0.0f);
    }
    else
    {
        cbLight.lightPosition = DirectX::XMFLOAT4(posFloat.x, posFloat.y, posFloat.z, 1.0f);
    }

    // Aktualisiere Direction
    DirectX::XMStoreFloat4(&cbLight.lightDirection, lookAt);

    // Kopiere die Daten in den lightBuffer
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = device->GetDeviceContext()->Map(lightBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0,
        &mappedResource);

    if (FAILED(hr)) {
        Debug::Log("Light.cpp: Failed to map light buffer");
        return;
    }

    memcpy(mappedResource.pData, &cbLight, sizeof(LightBufferData));
    device->GetDeviceContext()->Unmap(lightBuffer, 0);

    // Setze den lightBuffer im Shader
    device->GetDeviceContext()->VSSetConstantBuffers(1, 1, &lightBuffer);
    device->GetDeviceContext()->PSSetConstantBuffers(1, 1, &lightBuffer);
}