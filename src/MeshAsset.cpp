#include "MeshAsset.h"
#include "Surface.h"
#include <algorithm>

void MeshAsset::AddSlot(Surface* surface)
{
    if (!surface) return;

    m_slots.push_back(surface);
}

void MeshAsset::RemoveSlot(Surface* surface)
{
    // Tombstone: slot is set to nullptr instead of being erased from the vector.
    // This keeps all subsequent slot indices stable.
    // The render loop in RenderManager skips nullptr slots (if (!surface) continue).
    for (auto& slot : m_slots)
    {
        if (slot == surface)
        {
            slot = nullptr;
            return;
        }
    }
}

Surface* MeshAsset::GetSlot(unsigned int i) const
{
    if (i < m_slots.size())
        return m_slots[i];
    return nullptr;
}

bool MeshAsset::FindSlotIndex(const Surface* surface, unsigned int& outSlot) const
{
    if (!surface) return false;

    for (unsigned int i = 0; i < static_cast<unsigned int>(m_slots.size()); ++i)
    {
        if (m_slots[i] == surface)
        {
            outSlot = i;
            return true;
        }
    }

    return false;
}

unsigned int MeshAsset::NumSlots() const
{
    // Total slot count (including tombstoned nullptr slots).
    // Safe to use as an upper index bound.
    return static_cast<unsigned int>(m_slots.size());
}

unsigned int MeshAsset::NumActiveSlots() const
{
    // Non-empty slots only - useful for draw call prediction.
    unsigned int count = 0;
    for (const auto* s : m_slots)
        if (s) ++count;
    return count;
}
