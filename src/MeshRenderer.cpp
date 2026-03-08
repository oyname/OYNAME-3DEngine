#include "MeshRenderer.h"
#include "MeshAsset.h"
#include "Surface.h"
#include "Material.h"
#include <algorithm>

Material* MeshRenderer::GetMaterial(unsigned int slot, Material* standardMaterial) const
{
    if (slot < m_slotMaterials.size() && m_slotMaterials[slot] != nullptr)
        return m_slotMaterials[slot];

    return standardMaterial;
}

void MeshRenderer::SetMaterial(unsigned int slot, Material* mat)
{
    if (slot >= m_slotMaterials.size())
        m_slotMaterials.resize(slot + 1, nullptr);
    m_slotMaterials[slot] = mat;
}

void MeshRenderer::ClearMaterialReference(Material* material)
{
    if (!material) return;
    for (auto& slot : m_slotMaterials)
    {
        if (slot == material)
            slot = nullptr;
    }
}

unsigned int MeshRenderer::NumSlots() const noexcept
{
    return m_asset ? m_asset->NumSlots() : 0u;
}

static const std::vector<Surface*> s_emptySlots;

const std::vector<Surface*>& MeshRenderer::GetSlots() const noexcept
{
    return m_asset ? m_asset->GetSlots() : s_emptySlots;
}

Surface* MeshRenderer::GetSlot(unsigned int slot)
{
    if (!m_asset) return nullptr;
    if (slot >= m_asset->NumSlots()) return nullptr;
    return m_asset->GetSlot(slot);
}

const Surface* MeshRenderer::GetSlot(unsigned int slot) const
{
    if (!m_asset) return nullptr;
    if (slot >= m_asset->NumSlots()) return nullptr;
    return m_asset->GetSlot(slot);
}
