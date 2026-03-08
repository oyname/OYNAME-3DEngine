#pragma once
#include <vector>
#include <cstdint>

class MeshAsset;
class Material;
class Surface;
class Mesh;

// MeshRenderer: interne Renderdaten eines Meshs.
// Das MeshRenderer-Objekt selbst ist kein oeffentlicher Umbiegepunkt mehr.
// Asset- und Materialzuweisung laufen kontrolliert ueber Mesh/ObjectManager.
class MeshRenderer
{
public:
    Material* GetMaterial(unsigned int slot,
                          Material*      globalFallback) const;
    unsigned int NumSlots() const noexcept;
    const std::vector<Surface*>& GetSlots() const noexcept;
    const std::vector<Material*>& GetSlotMaterials() const noexcept { return m_slotMaterials; }
    bool         IsValid()  const noexcept { return m_asset != nullptr; }
    bool         HasAsset() const noexcept { return m_asset != nullptr; }
    const MeshAsset* GetAsset() const noexcept { return m_asset; }
    Surface* GetSlot(unsigned int slot);
    const Surface* GetSlot(unsigned int slot) const;

private:
    friend class Mesh;

    void SetMaterial(unsigned int slot, Material* mat);
    void ClearSlotMaterials() { m_slotMaterials.clear(); }
    void ClearMaterialReference(Material* material);
    void SetAsset(MeshAsset* asset) noexcept { m_asset = asset; }
    void ClearAsset() noexcept { m_asset = nullptr; }
    MeshAsset* AccessAsset() noexcept { return m_asset; }

    std::vector<Material*> m_slotMaterials;
    MeshAsset* m_asset = nullptr;
};
