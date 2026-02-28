#pragma once
#include <DirectXMath.h>
#include <cstdint>
#include "Transform.h"
#include "gdxutil.h"
#include "RenderLayers.h"
#include "Viewport.h"

// Forward declarations
class GDXDevice;
class Mesh;
class Camera;
class Light;
class EntityGpuData;

// Typ-Tag ersetzt dynamic_cast auf dem Hot Path.
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
    Transform   transform;
    MatrixSet   matrixSet;
    Viewport    viewport;

    // GPU-Ressourcen ausgelagert – Zugriff ueber entity->gpuData->...
    EntityGpuData* gpuData = nullptr;

public:
    explicit Entity(EntityType type = EntityType::Unknown);
    virtual ~Entity();

    virtual void Update(const GDXDevice* device);

    void GenerateViewMatrix(DirectX::XMVECTOR position,
        DirectX::XMVECTOR lookAt,
        DirectX::XMVECTOR up);

    void GenerateProjectionMatrix(float fieldOfView, float screenAspect,
        float nearZ, float farZ);

    void GenerateViewport(float TopLeftX, float TopLeftY,
        float Width, float Height,
        float MinDepth, float MaxDepth);

    void* operator new(size_t size)         { return _aligned_malloc(size, 16); }
    void  operator delete(void* p) noexcept { _aligned_free(p); }

    EntityType GetEntityType() const noexcept { return m_entityType; }
    bool IsMesh()   const noexcept { return m_entityType == EntityType::Mesh;   }
    bool IsCamera() const noexcept { return m_entityType == EntityType::Camera; }
    bool IsLight()  const noexcept { return m_entityType == EntityType::Light;  }

    inline Mesh*   AsMesh()   noexcept { return reinterpret_cast<Mesh*>(this);   }
    inline Camera* AsCamera() noexcept { return reinterpret_cast<Camera*>(this); }
    inline Light*  AsLight()  noexcept { return reinterpret_cast<Light*>(this);  }

    bool IsActive()  const noexcept { return m_active; }
    void SetActive(bool active)     noexcept { m_active = active; }

    bool IsVisible() const noexcept { return m_visible; }
    void SetVisible(bool visible)   noexcept { m_visible = visible; }

    bool GetCastShadows() const noexcept { return m_castShadows; }
    void SetCastShadows(bool enabled) noexcept { m_castShadows = enabled; }

    uint32_t GetLayerMask() const noexcept { return m_layerMask; }
    void     SetLayerMask(uint32_t mask) noexcept { m_layerMask = mask; }

protected:
    EntityType m_entityType = EntityType::Unknown;

    bool     m_active      = true;
    bool     m_visible     = true;
    bool     m_castShadows = true;
    uint32_t m_layerMask   = LAYER_DEFAULT;

    bool& isActive = m_active;
};

typedef Entity* LPENTITY;
typedef Entity  ENTITY;
