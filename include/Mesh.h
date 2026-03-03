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
// Die Geometriedaten und Materialzuweisungen sind in meshRenderer
// gekapselt. Mesh selbst enthaelt keine Surfaces direkt mehr – diese
// liegen im MeshAsset des MeshRenderer.
//
// Abwaertskompatibilitaet:
//   GetSurface(i)  -> meshRenderer.GetSlots()[i]
//   NumSurface()   -> meshRenderer.NumSlots()
//   pMaterial      -> meshRenderer.fallbackMaterial (Alias, identischer Speicher)
//   AddSurface()   -> leitet an meshRenderer.asset weiter.
class Mesh : public Entity
{
public:
    // Rendering-Komponente: haelt MeshAsset* + slotMaterials[].
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

    // ==================== ABWAERTSKOMPATIBLE ACCESSOREN ====================

    unsigned int NumSurface() const { return meshRenderer.NumSlots(); }
    Surface* GetSurface(unsigned int index);

    // Nur fuer ObjectManager!
    void AddSurface(Surface* surface);    // leitet an meshRenderer.asset weiter
    void RemoveSurface(Surface* surface); // entfernt aus meshRenderer.asset

    // pMaterial: Fallback-Material (== meshRenderer.fallbackMaterial).
    // Bleibt als direktes oeffentliches Mitglied, damit alter Code
    // unveraendert kompiliert.
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
