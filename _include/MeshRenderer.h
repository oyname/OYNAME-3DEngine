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
//   1. slotMaterials[i]    – per-Slot-Zuweisung via SurfaceMaterial()
//   2. meshFallback         – Mesh-weites Fallback (mesh->pMaterial)
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
    // Fallback-Kette: slotMaterials[i] -> meshFallback
    // surface-Parameter wird nur noch fuer kuenftige Erweiterungen mitgefuehrt.
    Material* GetMaterial(unsigned int slot,
                          const Surface* surface,
                          Material*      meshFallback) const;

    // Setzt das Material fuer einen bestimmten Slot.
    // Vergroessert slotMaterials automatisch, falls noetig.
    void SetMaterial(unsigned int slot, Material* mat);

    // Hilfsmethoden
    bool         IsValid()  const noexcept { return asset != nullptr; }
    unsigned int NumSlots() const noexcept;

    // Direktzugriff auf den Slot-Vektor des Assets fuer Range-based-for.
    const std::vector<Surface*>& GetSlots() const noexcept;
};
