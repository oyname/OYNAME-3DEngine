#pragma once
#include <vector>
#include <cstdint>

class MeshAsset;
class Material;
class Surface;

// MeshRenderer: Rendering-Komponente einer Entity.
//
// Verbindet ein MeshAsset (Geometrie) mit einem Material pro Slot.
// Ein MeshRenderer ohne gueltiges Asset rendert nichts.
//
// Materialaufloesung fuer Slot i (GetMaterial):
//   1. slotMaterials[i]       – neuer, bevorzugter Weg
//   2. surface->pMaterial     – Abwaertskompatibilitaet (per-Surface)
//   3. meshFallback           – entspricht dem frueheren Mesh::pMaterial
//
// Ownership: MeshRenderer besitzt weder das MeshAsset noch die Materialien.
// Alle Ressourcen liegen beim ObjectManager.
class MeshRenderer
{
public:
    // Geometriequelle (non-owning). Muss gesetzt sein, damit IsValid() true liefert.
    MeshAsset* asset = nullptr;

    // Material pro Slot (non-owning). Darf sparser als asset->NumSlots() sein.
    std::vector<Material*> slotMaterials;

    // Gibt das Material fuer Slot i zurueck.
    // Fallback-Kette: slotMaterials[i] -> surface->pMaterial -> meshFallback
    // surface und meshFallback duerfen nullptr sein.
    Material* GetMaterial(unsigned int slot,
                          const Surface* surface,
                          Material*      meshFallback) const;

    // Setzt das Material fuer einen bestimmten Slot direkt.
    // Vergroessert slotMaterials automatisch, falls noetig.
    void SetMaterial(unsigned int slot, Material* mat);

    // Hilfsmethoden
    bool         IsValid()   const noexcept { return asset != nullptr; }
    unsigned int NumSlots()  const noexcept;

    // Direktzugriff auf den Slot-Vektor des Assets fuer Range-based-for.
    // Gibt einen leeren Vektor zurueck, wenn kein Asset vorhanden ist.
    const std::vector<Surface*>& GetSlots() const noexcept;
};
