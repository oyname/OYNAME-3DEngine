#include "ObjectManager.h"
#include "MeshAsset.h"
#include <algorithm>
using namespace DirectX;

ObjectManager::ObjectManager() {}

ObjectManager::~ObjectManager()
{
    for (auto* shader : m_shaders)
        if (shader) shader->materials.clear();

    m_surfaces.clear();
    m_meshAssets.clear();
    m_meshes.clear();
    m_cameras.clear();
    m_lights.clear();
    m_materials.clear();
    m_shaders.clear();

    m_ownedSurfaces.clear();
    m_ownedMeshAssets.clear();
    m_ownedMeshes.clear();
    m_ownedCameras.clear();
    m_ownedLights.clear();
    m_ownedMaterials.clear();
    m_ownedShaders.clear();
}

bool ObjectManager::IsMeshAssetInUse(const MeshAsset* asset) const
{
    return CountMeshAssetUsers(asset) > 0;
}

unsigned int ObjectManager::CountMeshAssetUsers(const MeshAsset* asset) const
{
    if (!asset) return 0;

    unsigned int count = 0;
    for (const Mesh* mesh : m_meshes)
    {
        if (!mesh) continue;
        if (mesh->GetMeshAsset() == asset)
            ++count;
    }
    return count;
}

bool ObjectManager::DetachMeshAsset(Mesh* mesh, bool deleteOldIfUnused)
{
    if (!mesh) return false;

    MeshAsset* oldAsset = mesh->AccessMeshAssetInternal();
    mesh->DetachMeshAssetInternal();
    mesh->ClearSlotMaterialsInternal();

    if (deleteOldIfUnused && oldAsset && !IsMeshAssetInUse(oldAsset))
        DeleteMeshAsset(oldAsset);

    return true;
}

bool ObjectManager::SetMeshAsset(Mesh* mesh, MeshAsset* asset, bool deleteOldIfUnused)
{
    if (!mesh) return false;
    if (!asset)
    {
        DBLOG("objectmanager.cpp: SetMeshAsset - asset nullptr");
        return false;
    }

    if (mesh->GetMeshAsset() == asset)
        return true;

    MeshAsset* oldAsset = mesh->AccessMeshAssetInternal();
    mesh->SetMeshAssetInternal(asset);
    mesh->ClearSlotMaterialsInternal();

    if (deleteOldIfUnused && oldAsset && !IsMeshAssetInUse(oldAsset))
        DeleteMeshAsset(oldAsset);

    return true;
}

Surface* ObjectManager::CreateSurface()
{
    m_ownedSurfaces.push_back(std::make_unique<Surface>());
    Surface* surface = m_ownedSurfaces.back().get();
    m_surfaces.push_back(surface);
    return surface;
}

MeshAsset* ObjectManager::CreateMeshAsset()
{
    m_ownedMeshAssets.push_back(std::make_unique<MeshAsset>());
    MeshAsset* asset = m_ownedMeshAssets.back().get();
    m_meshAssets.push_back(asset);
    DBLOG("objectmanager.cpp: MeshAsset created");
    return asset;
}

Mesh* ObjectManager::CreateMesh()
{
    m_ownedMeshes.push_back(std::make_unique<Mesh>());
    Mesh* mesh = m_ownedMeshes.back().get();

    MeshAsset* asset = CreateMeshAsset();
    SetMeshAsset(mesh, asset, false);

    m_meshes.push_back(mesh);
    return mesh;
}

Camera* ObjectManager::CreateCamera()
{
    m_ownedCameras.push_back(std::make_unique<Camera>());
    Camera* camera = m_ownedCameras.back().get();
    m_cameras.push_back(camera);
    return camera;
}

Material* ObjectManager::CreateMaterial()
{
    m_ownedMaterials.push_back(std::make_unique<Material>());
    Material* material = m_ownedMaterials.back().get();
    material->id = ++m_nextMaterialId;
    m_materials.push_back(material);
    return material;
}

Shader* ObjectManager::CreateShader()
{
    m_ownedShaders.push_back(std::make_unique<Shader>());
    Shader* shader = m_ownedShaders.back().get();
    shader->id = ++m_nextShaderId;
    m_shaders.push_back(shader);
    return shader;
}

void ObjectManager::AddSurfaceToMesh(Mesh* mesh, Surface* surface)
{
    if (!mesh || !surface) return;
    mesh->AddSurface(surface);
}

void ObjectManager::AddMeshToMaterial(Material* material, Mesh* mesh)
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

void ObjectManager::SetSlotMaterial(Mesh* mesh, unsigned int slot, Material* material)
{
    if (!mesh || !material) return;

    mesh->SetSlotMaterialInternal(slot, material);

    if (material->pRenderShader)
        AssignShaderToMaterial(material->pRenderShader, material);
}

void ObjectManager::AddMaterialToShader(Shader* shader, Material* material)
{
    AssignShaderToMaterial(shader, material);
}

void ObjectManager::DeleteSurface(Surface* surface)
{
    if (!surface) return;

    for (auto* mesh : m_meshes)
        if (mesh) mesh->RemoveSurface(surface);

    m_surfaces.erase(std::remove(m_surfaces.begin(), m_surfaces.end(), surface), m_surfaces.end());
    RemoveOwned(m_ownedSurfaces, surface);
}

void ObjectManager::DeleteMesh(Mesh* mesh)
{
    if (!mesh) return;

    DetachMeshAsset(mesh, true);
    m_meshes.erase(std::remove(m_meshes.begin(), m_meshes.end(), mesh), m_meshes.end());
    RemoveOwned(m_ownedMeshes, mesh);
}

void ObjectManager::DeleteMeshAsset(MeshAsset* asset)
{
    if (!asset) return;

    const unsigned int users = CountMeshAssetUsers(asset);
    if (users > 0)
    {
        DBLOG("objectmanager.cpp: DeleteMeshAsset - asset still in use by ", (int)users, " mesh(es); delete refused");
        return;
    }

    m_meshAssets.erase(std::remove(m_meshAssets.begin(), m_meshAssets.end(), asset), m_meshAssets.end());
    if (RemoveOwned(m_ownedMeshAssets, asset))
        DBLOG("objectmanager.cpp: DeleteMeshAsset - asset deleted");
}

void ObjectManager::DeleteCamera(Camera* camera)
{
    m_cameras.erase(std::remove(m_cameras.begin(), m_cameras.end(), camera), m_cameras.end());
    RemoveOwned(m_ownedCameras, camera);
}

void ObjectManager::DeleteMaterial(Material* material)
{
    if (!material) return;

    for (auto* mesh : m_meshes)
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

void ObjectManager::RemoveSurfaceFromMesh(Mesh* mesh, Surface* surface)
{
    if (!mesh) return;
    mesh->RemoveSurface(surface);
}

void ObjectManager::RemoveMaterialFromShader(Shader* shader, Material* material)
{
    auto& materials = shader->materials;
    materials.erase(std::remove(materials.begin(), materials.end(), material), materials.end());
}

void ObjectManager::MoveSurface(Surface* surface, Mesh* from, Mesh* to)
{
    if (!surface || !to) return;

    unsigned int slot = 0;
    if (!from) return;

    from->RemoveSurface(surface);
    to->AddSurface(surface);
}

Surface* ObjectManager::GetPreviousSurface(Surface* currentSurface)
{
    for (auto it = m_surfaces.begin(); it != m_surfaces.end(); ++it)
    {
        if (*it == currentSurface)
        {
            if (it != m_surfaces.begin())
                return *std::prev(it);
            return nullptr;
        }
    }
    return nullptr;
}

Mesh* ObjectManager::GetPreviousMesh(Mesh* currentMesh)
{
    for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it)
    {
        if (*it == currentMesh)
        {
            if (it != m_meshes.begin())
                return *std::prev(it);
            return nullptr;
        }
    }
    return nullptr;
}

Camera* ObjectManager::GetPreviousCamera(Camera* currentCamera)
{
    for (auto it = m_cameras.begin(); it != m_cameras.end(); ++it)
    {
        if (*it == currentCamera)
        {
            if (it != m_cameras.begin())
                return *std::prev(it);
            return nullptr;
        }
    }
    return nullptr;
}

Material* ObjectManager::GetPreviousMaterial(Material* currentMaterial)
{
    for (auto it = m_materials.begin(); it != m_materials.end(); ++it)
    {
        if (*it == currentMaterial)
        {
            if (it != m_materials.begin())
                return *std::prev(it);
            return nullptr;
        }
    }
    return nullptr;
}

Shader* ObjectManager::GetPreviousShader(Shader* currentShader)
{
    for (auto it = m_shaders.begin(); it != m_shaders.end(); ++it)
    {
        if (*it == currentShader)
        {
            if (it != m_shaders.begin())
                return *std::prev(it);
            return nullptr;
        }
    }
    return nullptr;
}

Surface* ObjectManager::GetSurface(Mesh* mesh)
{
    if (!mesh || !mesh->HasMeshAsset()) return nullptr;
    return mesh->GetSurface(0);
}

Surface* ObjectManager::GetSurface(Mesh* mesh, unsigned int index)
{
    if (!mesh || !mesh->HasMeshAsset() || index >= mesh->NumSurface()) return nullptr;
    return mesh->GetSurface(index);
}

Material* ObjectManager::GetStandardMaterial() const
{
    if (m_defaultMaterial)
        return m_defaultMaterial;

    return m_materials.empty() ? nullptr : m_materials.front();
}

Shader* ObjectManager::GetShader(const Mesh& /*mesh*/) const
{
    Material* standard = GetStandardMaterial();
    return standard ? standard->pRenderShader : nullptr;
}

Shader* ObjectManager::GetShader(const Material& material) const
{
    return material.pRenderShader;
}

void ObjectManager::ProcessMesh()
{
    for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it)
    {
    }
}

void ObjectManager::DeleteShader(Shader* shader)
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

void ObjectManager::AssignShaderToMaterial(Shader* shader, Material* material)
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

static LightType ConvertD3DType(D3DLIGHTTYPE d3dType)
{
    return (d3dType == D3DLIGHT_POINT) ? LightType::Point : LightType::Directional;
}

Light* ObjectManager::CreateLight(D3DLIGHTTYPE type)
{
    return CreateLight(ConvertD3DType(type));
}

Light* ObjectManager::CreateLight(LightType type)
{
    if (m_lights.size() >= MAX_LIGHTS)
    {
        DBLOG("objectmanager.cpp: WARNING - MAX_LIGHTS (32) reached");
        return nullptr;
    }

    m_ownedLights.push_back(std::make_unique<Light>());
    Light* light = m_ownedLights.back().get();
    light->SetLightType(type);

    if (type == LightType::Point)
        light->SetRadius(100.0f);

    m_lights.push_back(light);
    DBLOG("objectmanager.cpp: Light created (total: ", static_cast<int>(m_lights.size()), ")");
    return light;
}

void ObjectManager::DeleteLight(Light* light)
{
    if (!light) return;
    m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), light), m_lights.end());
    RemoveOwned(m_ownedLights, light);
}
