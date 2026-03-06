#include "MeshRenderer.h"
#include "MeshAsset.h"
#include "Surface.h"
#include "Material.h"

Material* MeshRenderer::GetMaterial(unsigned int slot, Material* standardMaterial) const
{
    if (slot < slotMaterials.size() && slotMaterials[slot] != nullptr)
        return slotMaterials[slot];

    return standardMaterial;
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
