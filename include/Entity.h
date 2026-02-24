#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <cstdint>
#include "Transform.h"
#include "gdxutil.h"
#include "RenderLayers.h"

// Forward declarations
class GDXDevice;
class Mesh;
class Camera;
class Light;

// Typ-Tag ersetzt dynamic_cast auf dem Hot Path.
// Subklassen setzen m_entityType im Konstruktor.
enum class EntityType : uint8_t
{
    Unknown  = 0,
    Mesh     = 1,
    Camera   = 2,
    Light    = 3
};

class Entity
{
public:
    Transform      transform;
    MatrixSet      matrixSet;
    ID3D11Buffer*  constantBuffer;
    D3D11_VIEWPORT viewport;

public:
    explicit Entity(EntityType type = EntityType::Unknown);
    virtual ~Entity();

    // Basis-Update für Transform -> Matrix
    virtual void Update(const GDXDevice* device);

    // Matrix-Generierung für alle Entities
    void GenerateViewMatrix(DirectX::XMVECTOR position,
        DirectX::XMVECTOR lookAt,
        DirectX::XMVECTOR up);

    void GenerateProjectionMatrix(float fieldOfView, float screenAspect,
        float nearZ, float farZ);

    void GenerateViewport(float TopLeftX, float TopLeftY,
        float Width, float Height,
        float MinDepth, float MaxDepth);

    void* operator new(size_t size)        { return _aligned_malloc(size, 16); }
    void  operator delete(void* p) noexcept { _aligned_free(p); }

    // ---- Typ-System (kein dynamic_cast nötig) ----

    EntityType GetEntityType() const noexcept { return m_entityType; }
    bool IsMesh()   const noexcept { return m_entityType == EntityType::Mesh;   }
    bool IsCamera() const noexcept { return m_entityType == EntityType::Camera; }
    bool IsLight()  const noexcept { return m_entityType == EntityType::Light;  }

    // Schnelle Casts - nur aufrufen nachdem IsMesh()/IsCamera()/IsLight() geprueft wurde.
    // Kein RTTI, kein Overhead - reines Pointer-Reinterpret.
    inline Mesh*   AsMesh()   noexcept { return reinterpret_cast<Mesh*>(this);   }
    inline Camera* AsCamera() noexcept { return reinterpret_cast<Camera*>(this); }
    inline Light*  AsLight()  noexcept { return reinterpret_cast<Light*>(this);  }

    // ---- Active / Visible / Layer ----

    // active = false: kein Update(), kein Rendering
    bool IsActive()  const noexcept { return m_active; }
    void SetActive(bool active)     noexcept { m_active = active; }

    // visible = false: Update() läuft weiter, aber kein Rendering
    bool IsVisible() const noexcept { return m_visible; }
    void SetVisible(bool visible)   noexcept { m_visible = visible; }

    // castShadows = false: Mesh wirft keinen Schatten (Shadow Pass ueberspringt es)
    bool GetCastShadows() const noexcept { return m_castShadows; }
    void SetCastShadows(bool enabled) noexcept { m_castShadows = enabled; }

    // layerMask: welche Layer gehoert diese Entity an (Bitmask)
    uint32_t GetLayerMask() const noexcept { return m_layerMask; }
    void     SetLayerMask(uint32_t mask) noexcept { m_layerMask = mask; }

protected:
    EntityType m_entityType = EntityType::Unknown;

    bool     m_active      = true;
    bool     m_visible     = true;
    bool     m_castShadows = true;
    uint32_t m_layerMask   = LAYER_DEFAULT;

    // Rueckwaertskompatibilitaet: isActive als Alias
    bool& isActive = m_active;
};

typedef Entity* LPENTITY;
typedef Entity  ENTITY;
