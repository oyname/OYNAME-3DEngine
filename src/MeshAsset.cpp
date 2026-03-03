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
    // Der Render-Loop in RenderManager ueberspringt nullptr-Slots bereits
    // (if (!surface) continue;).
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

unsigned int MeshAsset::NumSlots() const
{
    // Gibt die Gesamtanzahl der Slots zurueck, einschliesslich leerer (nullptr) Slots.
    // Stabil als Index-Obergrenze verwendbar.
    return static_cast<unsigned int>(m_slots.size());
}

unsigned int MeshAsset::NumActiveSlots() const
{
    // Gibt die Anzahl nicht-leerer Slots zurueck.
    // Sinnvoll fuer Draw-Call-Vorhersage oder Validierung.
    unsigned int count = 0;
    for (const auto* s : m_slots)
        if (s) ++count;
    return count;
}
