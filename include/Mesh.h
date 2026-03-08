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
class MeshAsset;
class ObjectManager;

enum COLLISION {
    NONE = 0,
    BOX = 1,
    SPHERE = 2,
};

class Mesh : public Entity
{
public:
    DirectX::BoundingOrientedBox obb;

    // Skinning
    ID3D11Buffer* boneConstantBuffer = nullptr;  // Constant Buffer b4, owned
    bool          hasSkinning        = false;

public:
    Mesh();
    ~Mesh();

    void Update(const GDXDevice* device) override;
    void Update(const GDXDevice* device, const MatrixSet* matrixSet);

    unsigned int NumSurface() const { return m_meshRenderer.NumSlots(); }
    unsigned int GetSlotCount() const noexcept { return m_meshRenderer.NumSlots(); }
    bool HasSlot(unsigned int slot) const noexcept { return slot < m_meshRenderer.NumSlots() && m_meshRenderer.GetSlot(slot) != nullptr; }
    Surface* GetSurface(unsigned int index);
    bool TryGetSurfaceSlot(const Surface* surface, unsigned int& outSlot) const noexcept;
    const std::vector<Surface*>& GetSurfaces() const noexcept { return m_meshRenderer.GetSlots(); }
    bool HasMeshAsset() const noexcept { return m_meshRenderer.HasAsset(); }
    const MeshAsset* GetMeshAsset() const noexcept { return m_meshRenderer.GetAsset(); }
    MeshAsset* BorrowMeshAsset() const noexcept { return const_cast<MeshAsset*>(m_meshRenderer.GetAsset()); }
    Material* GetResolvedMaterial(unsigned int slot, Material* fallback) const { return m_meshRenderer.GetMaterial(slot, fallback); }
    const std::vector<Material*>& GetSlotMaterials() const noexcept { return m_meshRenderer.GetSlotMaterials(); }

    void AddSurface(Surface* surface);
    void RemoveSurface(Surface* surface);

    void SetCollisionMode(COLLISION collision);
    bool CheckCollision(Mesh* mesh);
    void CalculateOBB(unsigned int index);

    bool IsUpdatedThisFrame() const noexcept { return m_updatedThisFrame; }
    void MarkUpdated()              noexcept { m_updatedThisFrame = true; }
    void ResetFrameFlag()           noexcept { m_updatedThisFrame = false; }

    void* operator new(size_t size) { return _aligned_malloc(size, 16); }
    void  operator delete(void* p) noexcept { _aligned_free(p); }

private:
    friend class ObjectManager;

    void SetMeshAssetInternal(MeshAsset* asset) noexcept { m_meshRenderer.SetAsset(asset); }
    void DetachMeshAssetInternal() noexcept { m_meshRenderer.ClearAsset(); }
    MeshAsset* AccessMeshAssetInternal() noexcept { return m_meshRenderer.AccessAsset(); }
    void SetSlotMaterialInternal(unsigned int slot, Material* material) { m_meshRenderer.SetMaterial(slot, material); }
    void ClearSlotMaterialsInternal() { m_meshRenderer.ClearSlotMaterials(); }
    void ClearMaterialReferenceInternal(Material* material) { m_meshRenderer.ClearMaterialReference(material); }

    MeshRenderer m_meshRenderer;
    COLLISION collisionType      = COLLISION::NONE;
    bool      m_updatedThisFrame = false;
};

typedef Mesh* LPMESH;
