#pragma once
#include <d3d11.h>
#include <vector>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "Entity.h"
#include "MeshRenderer.h"
#include "gdxutil.h"

class Surface;
class Material;

enum COLLISION {
    NONE = 0,
    BOX = 1,
    SPHERE = 2,
};

// Mesh: Entity mit Transform und MeshRenderer-Komponente.
//
// Die Geometriedaten liegen im MeshAsset (mesh->meshRenderer.asset).
// Die Materialzuweisungen liegen im MeshRenderer (mesh->meshRenderer.slotMaterials).
//
// pMaterial: Mesh-weites Fallback-Material. Wird verwendet wenn fuer einen
// Slot kein spezifisches Material in slotMaterials eingetragen ist.
// Wird durch CreateMesh(..., material) gesetzt.
class Mesh : public Entity
{
public:
    MeshRenderer meshRenderer;

    DirectX::BoundingOrientedBox obb;

    // Skinning
    ID3D11Buffer* boneConstantBuffer = nullptr;  // Constant Buffer b4, owned
    bool          hasSkinning        = false;

public:
    Mesh();
    ~Mesh();

    void Update(const GDXDevice* device) override;
    void Update(const GDXDevice* device, const MatrixSet* matrixSet);

    // ==================== SURFACE-ZUGRIFF ====================

    unsigned int NumSurface() const { return meshRenderer.NumSlots(); }
    Surface* GetSurface(unsigned int index);

    // Nur fuer Mesh-interne Verwendung und ObjectManager!
    void AddSurface(Surface* surface);    // delegiert an meshRenderer.asset
    void RemoveSurface(Surface* surface);

    // Mesh-weites Fallback-Material.
    // Wird durch CreateMesh() gesetzt und von MeshRenderer::GetMaterial
    // als dritter Fallback verwendet, wenn kein Slot-Material vorhanden ist.
    Material* pMaterial = nullptr;

    // ==================== KOLLISION ====================
    void SetCollisionMode(COLLISION collision);
    bool CheckCollision(Mesh* mesh);
    void CalculateOBB(unsigned int index);

    // ==================== FRAME-UPDATE-FLAG ====================
    bool IsUpdatedThisFrame() const noexcept { return m_updatedThisFrame; }
    void MarkUpdated()              noexcept { m_updatedThisFrame = true; }
    void ResetFrameFlag()           noexcept { m_updatedThisFrame = false; }

    void* operator new(size_t size) { return _aligned_malloc(size, 16); }
    void  operator delete(void* p) noexcept { _aligned_free(p); }

private:
    COLLISION collisionType      = COLLISION::NONE;
    bool      m_updatedThisFrame = false;
};

typedef Mesh* LPMESH;
