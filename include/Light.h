#pragma once
#include "Entity.h"

// Light.h kennt kein ID3D11Buffer mehr.
// Der GPU-Constant-Buffer lebt in LightGpuData.
class LightGpuData;

__declspec(align(16))
struct LightBufferData
{
    DirectX::XMFLOAT4 lightPosition;
    DirectX::XMFLOAT4 lightDirection;
    DirectX::XMFLOAT4 lightDiffuseColor;
    DirectX::XMFLOAT4 lightAmbientColor;
};

enum class LightType
{
    Directional = 0,
    Point = 1
};

class Light : public Entity
{
public:
    Light();
    ~Light();

    virtual void Update(const GDXDevice* device) override;

    void UpdateLight(const GDXDevice* device, DirectX::XMVECTOR position, DirectX::XMVECTOR lookAt);
    void SetAmbientColor(const DirectX::XMFLOAT4& newColor);
    void SetDiffuseColor(const DirectX::XMFLOAT4& newColor);

    void SetLightType(LightType lightType);
    void SetRadius(float radius);
    void SetLightType(D3DLIGHTTYPE d3dType);

    LightType GetLightType() const { return lightType; }
    float GetRadius() const { return cbLight.lightDiffuseColor.w; }

    DirectX::XMMATRIX GetLightViewMatrix() const;
    DirectX::XMMATRIX GetLightProjectionMatrix() const;

    void SetShadowOrthoSize(float size);
    void SetShadowPlanes(float nearPlane, float farPlane);
    void SetShadowFov(float fovRadians);

public:
    LightBufferData  cbLight;
    LightType        lightType;

    // GPU-Ressourcen ausgelagert – Zugriff ueber light->gpuData->...
    LightGpuData* gpuData = nullptr;

private:
    float m_shadowOrthoSize = 50.0f;
    float m_shadowNear      = 0.1f;
    float m_shadowFar       = 1000.0f;
    float m_shadowFov       = DirectX::XM_PIDIV2;
};

typedef Light* LPLIGHT;
