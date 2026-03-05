#pragma once
#include <vector>
#include <cstdint>

class Surface;

// MeshAsset: Geometriedaten einer 3D-Ressource.
//
// Ein MeshAsset ist ein reines Geometrie-Datenobjekt ohne Transform, ohne
// Sichtbarkeits- oder Materialinformationen. Es haelt einen Satz von
// Surface-Slots (Sub-Geometrien), die jeweils eigene Vertex-/Indexdaten
// und einen GPU-Puffer besitzen.
//
// Ownership: MeshAsset besitzt seine Slots NICHT. Der ObjectManager
// verwaltet alle Surface-Instanzen zentral. MeshAsset haelt nur
// nicht-owning Zeiger.
//
// Sharing: Mehrere MeshRenderer-Instanzen (Entities) koennen dieselbe
// MeshAsset-Instanz referenzieren, sofern die Geometrie identisch ist
// (z. B. GPU-Instancing oder identische Props in einer Szene).
class MeshAsset
{
public:
    MeshAsset()  = default;
    ~MeshAsset() = default;

    // Fuegt einen Surface-Slot hinzu (non-owning).
    // Wird vom ObjectManager beim Aufbau des Meshes aufgerufen.
    void AddSlot(Surface* surface);

    // Entfernt einen Slot aus der Liste (loescht ihn nicht).
    void RemoveSlot(Surface* surface);

    // Gibt den Slot an Index i zurueck oder nullptr.
    Surface* GetSlot(unsigned int i) const;

    // Anzahl der Slots.
    unsigned int NumSlots() const;
    unsigned int NumActiveSlots() const;

    // Direktzugriff auf den internen Slot-Vektor (z. B. fuer Range-based-for).
    const std::vector<Surface*>& GetSlots() const noexcept { return m_slots; }

    bool IsEmpty() const noexcept { return m_slots.empty(); }

private:
    // Non-owning Zeiger auf die zugehoerigen Surface-Objekte.
    // Reihenfolge entspricht dem Slot-Index, der auch als Index
    // in MeshRenderer::slotMaterials dient.
    std::vector<Surface*> m_slots;
};
