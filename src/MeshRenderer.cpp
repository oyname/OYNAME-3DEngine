#include "MeshRenderer.h"
#include "MeshAsset.h"
#include "Surface.h"
#include "Material.h"

Material* MeshRenderer::GetMaterial(unsigned int slot,
                                    const Surface* surface,
                                    Material*      meshFallback) const
{
    // Schritt 1: Slot-spezifisches Material (neuer Weg)
    if (slot < slotMaterials.size() && slotMaterials[slot] != nullptr)
        return slotMaterials[slot];

    // Schritt 2: Material direkt an der Surface (Abwaertskompatibilitaet)
    if (surface && surface->pMaterial != nullptr)
        return surface->pMaterial;

    // Schritt 3: Fallback-Material des Mesh (entspricht fruehrem Mesh::pMaterial)
    return meshFallback;
}

void MeshRenderer::SetMaterial(unsigned int slot, Material* mat)
{
    if (slot >= slotMaterials.size())
        slotMaterials.resize(slot + 1, nullptr);
    slotMaterials[slot] = mat;
}

unsigned int MeshRenderer::NumSlots() const noexcept
{
    return asset ? asset->NumSlots() : 0u;
}

static const std::vector<Surface*> s_emptySlots;

const std::vector<Surface*>& MeshRenderer::GetSlots() const noexcept
{
    return asset ? asset->GetSlots() : s_emptySlots;
}
