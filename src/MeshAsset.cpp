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
    // Tombstone: Slot wird auf nullptr gesetzt, nicht aus dem Vektor entfernt.
    // Dadurch bleiben alle nachfolgenden Slot-Indizes stabil.
    // Der Render-Loop in RenderManager ueberspringt nullptr-Slots (if (!surface) continue).
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
    // Gesamtanzahl der Slots (inkl. tombstonter nullptr-Slots).
    // Stabil als Index-Obergrenze verwendbar.
    return static_cast<unsigned int>(m_slots.size());
}

unsigned int MeshAsset::NumActiveSlots() const
{
    // Nur nicht-leere Slots – sinnvoll fuer Draw-Call-Vorhersage.
    unsigned int count = 0;
    for (const auto* s : m_slots)
        if (s) ++count;
    return count;
}
