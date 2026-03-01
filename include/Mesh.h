#pragma once
#include <d3d11.h>
#include <vector>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "Entity.h"
#include "gdxutil.h"

class Surface;
class Material;

enum COLLISION {
    NONE = 0,
    BOX = 1,
    SPHERE = 2,
};

// Mesh besitzt NIEMALS Surfaces! Surfaces werden vom ObjectManager verwaltet.
// Mesh hat nur nicht-owning Pointer zu seinen Surfaces.
class Mesh : public Entity
{
public:
    std::vector<Surface*> m_surfaces;  // non-owning!
    Material* pMaterial = nullptr;      // non-owning
    DirectX::BoundingOrientedBox obb;

    // Skinning
    ID3D11Buffer* boneConstantBuffer = nullptr;  // Constant Buffer b4, owned
    bool          hasSkinning        = false;

public:
    Mesh();
    ~Mesh();

    void Update(const GDXDevice* device) override;
    void Update(const GDXDevice* device, const MatrixSet* matrixSet);

    unsigned int NumSurface() const { return static_cast<unsigned int>(m_surfaces.size()); }
    Surface* GetSurface(unsigned int index);

    // Nur für ObjectManager!
    void AddSurface(Surface* surface);      // non-owning
    void RemoveSurface(Surface* surface);   // nur Referenz entfernen

    void SetCollisionMode(COLLISION collision);
    bool CheckCollision(Mesh* mesh);
    void CalculateOBB(unsigned int index);

    // Frame-Update-Flag
    bool IsUpdatedThisFrame() const noexcept { return m_updatedThisFrame; }
    void MarkUpdated()              noexcept { m_updatedThisFrame = true; }
    void ResetFrameFlag()           noexcept { m_updatedThisFrame = false; }

    void* operator new(size_t size) { return _aligned_malloc(size, 16); }
    void  operator delete(void* p) noexcept { _aligned_free(p); }

private:
    COLLISION collisionType = COLLISION::NONE;
    bool      m_updatedThisFrame = false;
};

typedef Mesh* LPMESH;
