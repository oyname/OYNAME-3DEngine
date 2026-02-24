#pragma once
#include "Entity.h"
#include "gdxutil.h"
#include "gdxdevice.h"


__declspec(align(16))
struct LightBufferData
{
    DirectX::XMFLOAT4 lightPosition;     // XYZ: Position, W: 0 für direktional, 1 für positional
    DirectX::XMFLOAT4 lightDirection;    // XYZ: Direction (nur für direktionale Lichter)
    DirectX::XMFLOAT4 lightDiffuseColor; // RGB: Diffuse Farbe, A: Radius (für Point-Lights)
    DirectX::XMFLOAT4 lightAmbientColor; // RGB: Ambient Farbe (nur für erstes Licht)
};

// Lichts-Typen für einfacheren Code
enum class LightType
{
    Directional = 0,  // lightPosition.w = 0
    Point = 1         // lightPosition.w = 1
};

class Light : public Entity
{
public:
    Light();
    ~Light();

    // Override Entity::Update() - berechnet lightDirection automatisch aus Transform-Rotation
    virtual void Update(const GDXDevice* device) override;

    void UpdateLight(const GDXDevice* device, DirectX::XMVECTOR position, DirectX::XMVECTOR lookAt);
    void SetAmbientColor(const DirectX::XMFLOAT4& newColor);
    void SetDiffuseColor(const DirectX::XMFLOAT4& newColor);

    // Neue API: Moderne LightType enum
    void SetLightType(LightType lightType);
    void SetRadius(float radius);  // Radius für Point-Lights

    // Alte API (Rückwärtskompatibilität): D3DLIGHTTYPE wird zu LightType konvertiert
    void SetLightType(D3DLIGHTTYPE d3dType);

    // Getter für Light-Eigenschaften
    LightType GetLightType() const { return lightType; }
    float GetRadius() const { return cbLight.lightDiffuseColor.w; }

    // ==================== SHADOW MAPPING HELPERS ====================
    // View/Projection Matrizen für Shadow Mapping.
    // Directional -> Orthographic, Point -> Perspective.
    DirectX::XMMATRIX GetLightViewMatrix() const;
    DirectX::XMMATRIX GetLightProjectionMatrix() const;

    // Optional: Parameter für Shadow-Frustum
    void SetShadowOrthoSize(float size);
    void SetShadowPlanes(float nearPlane, float farPlane);
    void SetShadowFov(float fovRadians);

public:
    ID3D11Buffer* lightBuffer;
    LightBufferData cbLight;
    LightType lightType;

private:
    // Shadow-Frustum defaults (tuned for your shadow pass: square map, aspect = 1)
    float m_shadowOrthoSize = 50.0f;
    float m_shadowNear = 0.1f;
    float m_shadowFar = 1000.0f;
    float m_shadowFov = DirectX::XM_PIDIV2; // 90° for point/spot style shadow projections
};

typedef Light* LPLIGHT;