#include "AssetManager.h"

#include "Surface.h"
#include "MeshAsset.h"
#include "Material.h"
#include "Shader.h"
#include "Mesh.h"
#include "Scene.h"

AssetManager::~AssetManager()
{
    for (auto* shader : m_shaders)
        if (shader) shader->materials.clear();
}

bool AssetManager::IsMeshAssetInUse(const Scene& scene, const MeshAsset* asset) const
{
    return CountMeshAssetUsers(scene, asset) > 0;
}

unsigned int AssetManager::CountMeshAssetUsers(const Scene& scene, const MeshAsset* asset) const
{
    if (!asset) return 0;

    unsigned int count = 0;
    for (const Mesh* mesh : scene.GetMeshes())
    {
        if (!mesh) continue;
        if (mesh->GetMeshAsset() == asset)
            ++count;
    }
    return count;
}

Surface* AssetManager::CreateSurface()
{
    m_ownedSurfaces.push_back(std::make_unique<Surface>());
    Surface* surface = m_ownedSurfaces.back().get();
    m_surfaces.push_back(surface);
    return surface;
}

MeshAsset* AssetManager::CreateMeshAsset()
{
    m_ownedMeshAssets.push_back(std::make_unique<MeshAsset>());
    MeshAsset* asset = m_ownedMeshAssets.back().get();
    m_meshAssets.push_back(asset);
    DBLOG("AssetManager.cpp: MeshAsset created");
    return asset;
}

Material* AssetManager::CreateMaterial()
{
    m_ownedMaterials.push_back(std::make_unique<Material>());
    Material* material = m_ownedMaterials.back().get();
    material->id = ++m_nextMaterialId;
    m_materials.push_back(material);
    return material;
}

Shader* AssetManager::CreateShader()
{
    m_ownedShaders.push_back(std::make_unique<Shader>());
    Shader* shader = m_ownedShaders.back().get();
    shader->id = ++m_nextShaderId;
    m_shaders.push_back(shader);
    return shader;
}

Mesh* AssetManager::CreateManagedMesh(Scene& scene)
{
    Mesh* mesh = scene.CreateMesh();
    if (!mesh) return nullptr;

    MeshAsset* asset = CreateMeshAsset();
    if (!asset)
    {
        scene.DeleteMesh(mesh);
        return nullptr;
    }

    if (!SetMeshAsset(scene, mesh, asset, false))
    {
        scene.DeleteMesh(mesh);
        DeleteMeshAsset(scene, asset);
        return nullptr;
    }

    return mesh;
}

void AssetManager::DeleteManagedMesh(Scene& scene, Mesh* mesh)
{
    if (!mesh) return;
    DetachMeshAsset(scene, mesh, true);
    scene.DeleteMesh(mesh);
}

bool AssetManager::DetachMeshAsset(Scene& scene, Mesh* mesh, bool deleteOldIfUnused)
{
    if (!mesh) return false;

    MeshAsset* oldAsset = mesh->AccessMeshAssetInternal();
    mesh->DetachMeshAssetInternal();
    mesh->ClearSlotMaterialsInternal();

    if (deleteOldIfUnused && oldAsset && !IsMeshAssetInUse(scene, oldAsset))
        DeleteMeshAsset(scene, oldAsset);

    return true;
}

bool AssetManager::SetMeshAsset(Scene& scene, Mesh* mesh, MeshAsset* asset, bool deleteOldIfUnused)
{
    if (!mesh) return false;
    if (!asset)
    {
        DBLOG("AssetManager.cpp: SetMeshAsset - asset nullptr");
        return false;
    }

    if (mesh->GetMeshAsset() == asset)
        return true;

    MeshAsset* oldAsset = mesh->AccessMeshAssetInternal();
    mesh->SetMeshAssetInternal(asset);
    mesh->ClearSlotMaterialsInternal();

    if (deleteOldIfUnused && oldAsset && !IsMeshAssetInUse(scene, oldAsset))
        DeleteMeshAsset(scene, oldAsset);

    return true;
}

void AssetManager::AddSurfaceToMesh(Mesh* mesh, Surface* surface)
{
    if (!mesh || !surface) return;
    mesh->AddSurface(surface);
}

void AssetManager::AddMeshToMaterial(Material* material, Mesh* mesh)
{
    if (!material || !mesh) return;

    const auto& slots = mesh->GetSurfaces();
    for (unsigned int i = 0; i < (unsigned int)slots.size(); ++i)
    {
        if (!slots[i]) continue;
        mesh->SetSlotMaterialInternal(i, material);
    }

    if (material->pRenderShader)
        AssignShaderToMaterial(material->pRenderShader, material);
}

void AssetManager::SetSlotMaterial(Mesh* mesh, unsigned int slot, Material* material)
{
    if (!mesh || !material) return;

    mesh->SetSlotMaterialInternal(slot, material);

    if (material->pRenderShader)
        AssignShaderToMaterial(material->pRenderShader, material);
}

void AssetManager::AddMaterialToShader(Shader* shader, Material* material)
{
    AssignShaderToMaterial(shader, material);
}

void AssetManager::DeleteSurface(Scene& scene, Surface* surface)
{
    if (!surface) return;

    for (auto* mesh : scene.GetMeshes())
        if (mesh) mesh->RemoveSurface(surface);

    m_surfaces.erase(std::remove(m_surfaces.begin(), m_surfaces.end(), surface), m_surfaces.end());
    RemoveOwned(m_ownedSurfaces, surface);
}

void AssetManager::DeleteMeshAsset(Scene& scene, MeshAsset* asset)
{
    if (!asset) return;

    const unsigned int users = CountMeshAssetUsers(scene, asset);
    if (users > 0)
    {
        DBLOG("AssetManager.cpp: DeleteMeshAsset - asset still in use by ", (int)users, " mesh(es); delete refused");
        return;
    }

    m_meshAssets.erase(std::remove(m_meshAssets.begin(), m_meshAssets.end(), asset), m_meshAssets.end());
    if (RemoveOwned(m_ownedMeshAssets, asset))
        DBLOG("AssetManager.cpp: DeleteMeshAsset - asset deleted");
}

void AssetManager::DeleteMaterial(Scene& scene, Material* material)
{
    if (!material) return;

    for (auto* mesh : scene.GetMeshes())
        if (mesh) mesh->ClearMaterialReferenceInternal(material);

    Shader* sh = material->pRenderShader;
    if (sh)
    {
        auto& v = sh->materials;
        v.erase(std::remove(v.begin(), v.end(), material), v.end());
    }

    if (m_defaultMaterial == material)
        m_defaultMaterial = nullptr;

    m_materials.erase(std::remove(m_materials.begin(), m_materials.end(), material), m_materials.end());
    RemoveOwned(m_ownedMaterials, material);
}

void AssetManager::DeleteShader(Shader* shader)
{
    if (!shader) return;

    for (auto* mat : shader->materials)
    {
        if (mat && mat->pRenderShader == shader)
            mat->pRenderShader = nullptr;
    }
    shader->materials.clear();

    m_shaders.erase(std::remove(m_shaders.begin(), m_shaders.end(), shader), m_shaders.end());
    RemoveOwned(m_ownedShaders, shader);
}

void AssetManager::RemoveSurfaceFromMesh(Mesh* mesh, Surface* surface)
{
    if (!mesh) return;
    mesh->RemoveSurface(surface);
}

void AssetManager::RemoveMaterialFromShader(Shader* shader, Material* material)
{
    if (!shader) return;
    auto& materials = shader->materials;
    materials.erase(std::remove(materials.begin(), materials.end(), material), materials.end());
}

void AssetManager::MoveSurface(Surface* surface, Mesh* from, Mesh* to)
{
    if (!surface || !from || !to) return;
    from->RemoveSurface(surface);
    to->AddSurface(surface);
}

Surface* AssetManager::GetPreviousSurface(Surface* currentSurface)
{
    for (auto it = m_surfaces.begin(); it != m_surfaces.end(); ++it)
    {
        if (*it == currentSurface)
            return (it != m_surfaces.begin()) ? *std::prev(it) : nullptr;
    }
    return nullptr;
}

Material* AssetManager::GetPreviousMaterial(Material* currentMaterial)
{
    for (auto it = m_materials.begin(); it != m_materials.end(); ++it)
    {
        if (*it == currentMaterial)
            return (it != m_materials.begin()) ? *std::prev(it) : nullptr;
    }
    return nullptr;
}

Shader* AssetManager::GetPreviousShader(Shader* currentShader)
{
    for (auto it = m_shaders.begin(); it != m_shaders.end(); ++it)
    {
        if (*it == currentShader)
            return (it != m_shaders.begin()) ? *std::prev(it) : nullptr;
    }
    return nullptr;
}

Surface* AssetManager::GetSurface(Mesh* mesh)
{
    if (!mesh || !mesh->HasMeshAsset()) return nullptr;
    return mesh->GetSurface(0);
}

Surface* AssetManager::GetSurface(Mesh* mesh, unsigned int index)
{
    if (!mesh || !mesh->HasMeshAsset() || index >= mesh->NumSurface()) return nullptr;
    return mesh->GetSurface(index);
}

Material* AssetManager::GetStandardMaterial() const
{
    if (m_defaultMaterial)
        return m_defaultMaterial;

    return m_materials.empty() ? nullptr : m_materials.front();
}

Shader* AssetManager::GetShader(const Mesh& mesh) const
{
    if (!mesh.HasMeshAsset() || mesh.GetSlotCount() == 0)
        return nullptr;

    Material* material = mesh.GetResolvedMaterial(0, GetStandardMaterial());
    return material ? material->pRenderShader : nullptr;
}

Shader* AssetManager::GetShader(const Material& material) const
{
    return material.pRenderShader;
}

void AssetManager::AssignShaderToMaterial(Shader* shader, Material* material)
{
    if (!material) return;

    Shader* old = material->pRenderShader;
    if (old && old != shader)
    {
        auto& v = old->materials;
        v.erase(std::remove(v.begin(), v.end(), material), v.end());
    }

    material->pRenderShader = shader;

    if (shader)
    {
        auto& v = shader->materials;
        if (std::find(v.begin(), v.end(), material) == v.end())
            v.push_back(material);
    }
}
