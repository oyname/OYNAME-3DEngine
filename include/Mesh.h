#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "Entity.h"
#include "gdxutil.h"
#include "Surface.h"

class Material;

enum COLLISION {
    NONE = 0,
    BOX = 1,
    SPHERE = 2,
};

enum class RenderQueueType { Opaque, AlphaTest, Transparent, Additive };

class Mesh : public Entity
{
public:
    std::vector<Surface*> surfaces;
    Material* pMaterial = nullptr;
    DirectX::BoundingOrientedBox obb;

public:
    Mesh();
    ~Mesh();

    // 1. Ueberschreibt Entity::Update() - fuer einfaches Update
    void Update(const GDXDevice* device) override;

    // 2. Rendering-spezifisches Update mit MatrixSet
    void Update(const GDXDevice* device, const MatrixSet* matrixSet);

    unsigned int NumSurface();
    Surface* GetSurface(unsigned int index);
    void         AddSurfaceToMesh(Surface* surface);

    void SetCollisionMode(COLLISION collision);
    bool CheckCollision(Mesh* mesh);
    void CalculateOBB(unsigned int index);

    // Frame-Update-Flag: verhindert mehrfaches Update desselben Mesh
    // wenn mehrere Surfaces auf dasselbe Mesh zeigen.
    // ResetFrameFlag() wird einmal pro Frame in BuildRenderQueue aufgerufen.
    bool IsUpdatedThisFrame() const noexcept { return m_updatedThisFrame; }
    void MarkUpdated()              noexcept { m_updatedThisFrame = true; }
    void ResetFrameFlag()           noexcept { m_updatedThisFrame = false; }

    void* operator new(size_t size) { return _aligned_malloc(size, 16); }
    void  operator delete(void* p) noexcept { _aligned_free(p); }

private:
    COLLISION collisionType;
    bool      m_updatedThisFrame = false;
};

typedef Mesh* LPMESH;
