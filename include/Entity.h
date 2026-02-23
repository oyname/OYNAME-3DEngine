#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <cstdint>
#include "Transform.h"
#include "gdxutil.h"
#include "RenderLayers.h"

// Forward declaration
class GDXDevice;

class Entity
{
public:
    Transform      transform;
    MatrixSet      matrixSet;
    ID3D11Buffer*  constantBuffer;
    D3D11_VIEWPORT viewport;

public:
    Entity();
    virtual ~Entity();

    // Basis-Update für Transform → Matrix
    virtual void Update(const GDXDevice* device);

    // Matrix-Generierung für alle Entities
    void GenerateViewMatrix(DirectX::XMVECTOR position,
                            DirectX::XMVECTOR lookAt,
                            DirectX::XMVECTOR up);

    void GenerateProjectionMatrix(float fieldOfView, float screenAspect,
                                  float nearZ, float farZ);

    void GenerateViewport(float TopLeftX, float TopLeftY,
                          float Width,    float Height,
                          float MinDepth, float MaxDepth);

    void* operator new(size_t size)      { return _aligned_malloc(size, 16); }
    void  operator delete(void* p) noexcept { _aligned_free(p); }

    // ---- Active / Visible / Layer ----

    // active = false: kein Update(), kein Rendering
    bool IsActive()  const noexcept { return m_active; }
    void SetActive(bool active)     noexcept { m_active  = active; }

    // visible = false: Update() läuft weiter, aber kein Rendering
    bool IsVisible() const noexcept { return m_visible; }
    void SetVisible(bool visible)   noexcept { m_visible = visible; }

    // layerMask: welche Layer gehört diese Entity an (Bitmask)
    uint32_t GetLayerMask() const noexcept          { return m_layerMask; }
    void     SetLayerMask(uint32_t mask) noexcept   { m_layerMask = mask; }

protected:
    bool     m_active    = true;
    bool     m_visible   = true;
    uint32_t m_layerMask = LAYER_DEFAULT;

    // Rückwärtskompatibilität: isActive als Alias
    // (alles was bisher isActive nutzte bleibt kompilierbar)
    bool& isActive = m_active;
};

typedef Entity* LPENTITY;
typedef Entity  ENTITY;
