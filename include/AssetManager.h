#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "gdxutil.h"
#include "Surface.h"
#include "MeshAsset.h"
#include "Material.h"
#include "Shader.h"

class Scene;
class Mesh;

class AssetManager
{
public:
    AssetManager() = default;
    ~AssetManager();

    void Init() {}

    Surface* CreateSurface();
    MeshAsset* CreateMeshAsset();
    Material* CreateMaterial();
    Shader* CreateShader();

    void DeleteSurface(Scene& scene, Surface* surface);
    void DeleteMeshAsset(Scene& scene, MeshAsset* asset);
    void DeleteMaterial(Scene& scene, Material* material);
    void DeleteShader(Shader* shader);

    bool SetMeshAsset(Scene& scene, Mesh* mesh, MeshAsset* asset, bool deleteOldIfUnused = true);
    bool DetachMeshAsset(Scene& scene, Mesh* mesh, bool deleteOldIfUnused = true);
    void AddSurfaceToMesh(Mesh* mesh, Surface* surface);
    void SetSlotMaterial(Mesh* mesh, unsigned int slot, Material* material);
    void AddMeshToMaterial(Material* material, Mesh* mesh);
    void AssignShaderToMaterial(Shader* shader, Material* material);
    void AddMaterialToShader(Shader* shader, Material* material);
    void RemoveSurfaceFromMesh(Mesh* mesh, Surface* surface);
    void RemoveMaterialFromShader(Shader* shader, Material* material);
    void MoveSurface(Surface* surface, Mesh* from, Mesh* to);

    Surface* GetPreviousSurface(Surface* currentSurface);
    Material* GetPreviousMaterial(Material* currentMaterial);
    Shader* GetPreviousShader(Shader* currentShader);
    Surface* GetSurface(Mesh* mesh);
    Surface* GetSurface(Mesh* mesh, unsigned int index);
    Material* GetStandardMaterial() const;
    Shader* GetShader(const Mesh& mesh) const;
    Shader* GetShader(const Material& material) const;

    const std::vector<Shader*>& GetShaders() const noexcept { return m_shaders; }
    const std::vector<Material*>& GetMaterials() const noexcept { return m_materials; }
    const std::vector<MeshAsset*>& GetMeshAssets() const noexcept { return m_meshAssets; }
    const std::vector<Surface*>& GetSurfaces() const noexcept { return m_surfaces; }

    void SetDefaultMaterial(Material* material) { m_defaultMaterial = material; }

    Mesh* CreateManagedMesh(Scene& scene);
    void DeleteManagedMesh(Scene& scene, Mesh* mesh);

private:
    template<typename T>
    static bool RemoveOwned(std::vector<std::unique_ptr<T>>& owner, T* ptr)
    {
        auto it = std::find_if(owner.begin(), owner.end(),
            [ptr](const std::unique_ptr<T>& p) { return p.get() == ptr; });
        if (it == owner.end()) return false;
        owner.erase(it);
        return true;
    }

    bool IsMeshAssetInUse(const Scene& scene, const MeshAsset* asset) const;
    unsigned int CountMeshAssetUsers(const Scene& scene, const MeshAsset* asset) const;

    uint32_t m_nextShaderId = 0;
    uint32_t m_nextMaterialId = 0;

    std::vector<Surface*>   m_surfaces;
    std::vector<MeshAsset*> m_meshAssets;
    std::vector<Material*>  m_materials;
    std::vector<Shader*>    m_shaders;

    std::vector<std::unique_ptr<Surface>>   m_ownedSurfaces;
    std::vector<std::unique_ptr<MeshAsset>> m_ownedMeshAssets;
    std::vector<std::unique_ptr<Material>>  m_ownedMaterials;
    std::vector<std::unique_ptr<Shader>>    m_ownedShaders;

    Material* m_defaultMaterial = nullptr;
};
