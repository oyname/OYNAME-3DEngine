#include "ObjectManager.h"
#include "MeshAsset.h"
#include "Memory.h"
#include <algorithm>
using namespace DirectX;

ObjectManager::ObjectManager() {}

ObjectManager::~ObjectManager()
{
    // Container clearen, bevor Objekte geloescht werden
    for (auto& shader : m_shaders)
        shader->materials.clear();

    for (auto& mesh : m_meshes)
    {
        // Slots aus dem Asset loeschen (Referenzen; Surfaces leben in m_surfaces)
        if (mesh->meshRenderer.asset)
            mesh->meshRenderer.asset->GetSlots(); // lesend – nur sicherstellen, kein Clear
        mesh->pMaterial = nullptr;
    }

    // Surfaces loeschen
    for (auto& surface : m_surfaces)
        Memory::SafeDelete(surface);
    m_surfaces.clear();

    // MeshAssets loeschen (nach Surfaces, da Assets nur Zeiger halten)
    for (auto& asset : m_meshAssets)
        Memory::SafeDelete(asset);
    m_meshAssets.clear();

    // Meshes loeschen
    for (auto& mesh : m_meshes)
        Memory::SafeDelete(mesh);
    m_meshes.clear();

    // Kameras loeschen
    for (auto& camera : m_cameras)
        Memory::SafeDelete(camera);
    m_cameras.clear();

    // Materialien loeschen
    for (auto& material : m_materials)
        Memory::SafeDelete(material);
    m_materials.clear();

    // Shader loeschen
    for (auto& shader : m_shaders)
        Memory::SafeDelete(shader);
    m_shaders.clear();

    // Entity-Liste clearen (enthaelt nur Referenzen)
    m_entities.clear();
}

// ==================== CREATE ====================

Surface* ObjectManager::CreateSurface()
{
    Surface* surface = new Surface;
    m_surfaces.push_back(surface);
    return surface;
}

MeshAsset* ObjectManager::CreateMeshAsset()
{
    MeshAsset* asset = new MeshAsset;
    m_meshAssets.push_back(asset);
    Debug::Log("objectmanager.cpp: MeshAsset erstellt");
    return asset;
}

Mesh* ObjectManager::CreateMesh()
{
    Mesh* mesh = new Mesh;

    // Jedes neue Mesh bekommt automatisch ein eigenes MeshAsset.
    // Fuer Instancing kann das Asset spaeter durch ein geteiltes ersetzt werden.
    MeshAsset* asset = CreateMeshAsset();
    mesh->meshRenderer.asset = asset;

    m_meshes.push_back(mesh);
    m_entities.push_back(mesh);
    return mesh;
}

Camera* ObjectManager::CreateCamera()
{
    Camera* camera = new Camera;
    m_cameras.push_back(camera);
    m_entities.push_back(camera);
    return camera;
}

Material* ObjectManager::CreateMaterial()
{
    Material* material = new Material;
    m_materials.push_back(material);
    return material;
}

Shader* ObjectManager::CreateShader()
{
    Shader* shader = new Shader;
    m_shaders.push_back(shader);
    return shader;
}

// ==================== REGISTER / UNREGISTER ====================

void ObjectManager::RegisterRenderable(Mesh* mesh)
{
    if (!mesh) return;
    m_renderMeshes.push_back(mesh);
}

void ObjectManager::UnregisterRenderable(Mesh* mesh)
{
    if (!mesh) return;
    m_renderMeshes.erase(
        std::remove(m_renderMeshes.begin(), m_renderMeshes.end(), mesh),
        m_renderMeshes.end());
}

// ==================== ADD ====================

void ObjectManager::AddSurfaceToMesh(Mesh* mesh, Surface* surface)
{
    if (!mesh || !surface) return;

    surface->pMesh = mesh;
    mesh->AddSurface(surface);  // delegiert an meshRenderer.asset
}

void ObjectManager::AddMaterialToSurface(Material* material, Surface* surface)
{
    if (!material || !surface) return;
    surface->pMaterial = material;

    if (material->pRenderShader)
        AssignShaderToMaterial(material->pRenderShader, material);
}

void ObjectManager::AddMeshToMaterial(Material* material, Mesh* mesh)
{
    if (!material || !mesh) return;

    // Setzt das Fallback-Material des Mesh (meshRenderer wird intern ueber pMaterial informiert)
    mesh->pMaterial = material;

    if (material->pRenderShader)
        AssignShaderToMaterial(material->pRenderShader, material);
}

void ObjectManager::SetSlotMaterial(Mesh* mesh, unsigned int slot, Material* material)
{
    if (!mesh || !material) return;

    mesh->meshRenderer.SetMaterial(slot, material);

    if (material->pRenderShader)
        AssignShaderToMaterial(material->pRenderShader, material);
}

void ObjectManager::AddMaterialToShader(Shader* shader, Material* material)
{
    AssignShaderToMaterial(shader, material);
}

// ==================== DELETE ====================

void ObjectManager::DeleteSurface(Surface* surface)
{
    if (!surface) return;

    // Aus MeshAsset des Besitzer-Mesh entfernen
    if (surface->GetOwner())
    {
        surface->GetOwner()->RemoveSurface(surface);
    }
    else
    {
        // Fallback: alle Meshes durchsuchen
        for (auto& mesh : m_meshes)
            mesh->RemoveSurface(surface);
    }

    auto it = std::find(m_surfaces.begin(), m_surfaces.end(), surface);
    if (it != m_surfaces.end())
    {
        m_surfaces.erase(it);
        Memory::SafeDelete(surface);
    }
}

void ObjectManager::DeleteMesh(Mesh* mesh)
{
    if (!mesh) return;

    auto entIt = std::find(m_entities.begin(), m_entities.end(), mesh);
    if (entIt != m_entities.end())
        m_entities.erase(entIt);

    UnregisterRenderable(mesh);

    // Surfaces vom Asset trennen (nicht loeschen – ObjectManager loescht sie separat)
    if (mesh->meshRenderer.asset)
    {
        const auto& slots = mesh->meshRenderer.asset->GetSlots();
        for (Surface* s : slots)
        {
            if (s)
            {
                s->SetOwner(nullptr);
                s->pMesh = nullptr;
            }
        }
        // Asset mitloeschen, wenn es exklusiv ist (Normalfall: CreateMesh legt immer
        // ein eigenes Asset an). Geteilte Assets muessen vorher manuell getrennt werden:
        //   mesh->meshRenderer.asset = nullptr;  vor DeleteMesh aufrufen.
        MeshAsset* ownedAsset = mesh->meshRenderer.asset;
        mesh->meshRenderer.asset = nullptr;

        auto assetIt = std::find(m_meshAssets.begin(), m_meshAssets.end(), ownedAsset);
        if (assetIt != m_meshAssets.end())
        {
            m_meshAssets.erase(assetIt);
            Memory::SafeDelete(ownedAsset);
            Debug::Log("objectmanager.cpp: DeleteMesh - MeshAsset mitgeloescht");
        }
    }

    mesh->pMaterial = nullptr;

    auto it = std::find(m_meshes.begin(), m_meshes.end(), mesh);
    if (it != m_meshes.end())
    {
        m_meshes.erase(it);
        Memory::SafeDelete(mesh);
    }
}

void ObjectManager::DeleteMeshAsset(MeshAsset* asset)
{
    if (!asset) return;
    auto it = std::find(m_meshAssets.begin(), m_meshAssets.end(), asset);
    if (it != m_meshAssets.end())
    {
        m_meshAssets.erase(it);
        Memory::SafeDelete(asset);
    }
}

void ObjectManager::DeleteCamera(Camera* camera)
{
    auto entIt = std::find(m_entities.begin(), m_entities.end(), camera);
    if (entIt != m_entities.end())
        m_entities.erase(entIt);

    auto it = std::find(m_cameras.begin(), m_cameras.end(), camera);
    if (it != m_cameras.end())
    {
        m_cameras.erase(it);
        Memory::SafeDelete(camera);
    }
}

void ObjectManager::DeleteMaterial(Material* material)
{
    if (!material) return;

    // Meshes, die dieses Material als Fallback nutzen → nullen
    for (auto* mesh : m_meshes)
    {
        if (mesh && mesh->pMaterial == material)
            mesh->pMaterial = nullptr;

        // Auch aus slotMaterials entfernen
        for (auto& slot : mesh->meshRenderer.slotMaterials)
        {
            if (slot == material)
                slot = nullptr;
        }
    }

    // Surfaces, die dieses Material direkt nutzen → nullen
    for (auto* surface : m_surfaces)
    {
        if (surface && surface->pMaterial == material)
            surface->pMaterial = nullptr;
    }

    // Aus Shader-Bucket entfernen
    Shader* sh = material->pRenderShader;
    if (sh)
    {
        auto& v = sh->materials;
        v.erase(std::remove(v.begin(), v.end(), material), v.end());
    }

    auto it = std::find(m_materials.begin(), m_materials.end(), material);
    if (it != m_materials.end())
    {
        m_materials.erase(it);
        Memory::SafeDelete(material);
    }
}

// ==================== REMOVE ====================

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

    from = surface->GetOwner();
    if (!from) return;

    from->RemoveSurface(surface);
    to->AddSurface(surface);
}

// ==================== GET PREVIOUS ====================

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

// ==================== GET ====================

Surface* ObjectManager::GetSurface(Mesh* mesh)
{
    if (!mesh || !mesh->meshRenderer.asset) return nullptr;
    return mesh->meshRenderer.asset->GetSlot(0);
}

Material* ObjectManager::GetStandardMaterial() const
{
    return m_materials.empty() ? nullptr : m_materials.front();
}

Shader* ObjectManager::GetShader(const Surface& surface) const
{
    if (surface.pMesh && surface.pMesh->pMaterial)
        return surface.pMesh->pMaterial->pRenderShader;
    return nullptr;
}

Shader* ObjectManager::GetShader(const Mesh& mesh) const
{
    return mesh.pMaterial ? mesh.pMaterial->pRenderShader : nullptr;
}

Shader* ObjectManager::GetShader(const Material& material) const
{
    return material.pRenderShader;
}

void ObjectManager::ProcessMesh()
{
    for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it)
    {
        // Processing logic here
    }
}

// ==================== SHADER ====================

void ObjectManager::DeleteShader(Shader* shader)
{
    if (!shader) return;

    for (auto* mat : shader->materials)
    {
        if (mat && mat->pRenderShader == shader)
            mat->pRenderShader = nullptr;
    }
    shader->materials.clear();

    auto it = std::find(m_shaders.begin(), m_shaders.end(), shader);
    if (it != m_shaders.end())
    {
        m_shaders.erase(it);
        Memory::SafeDelete(shader);
    }
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

// ==================== LIGHT ====================

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
        Debug::Log("objectmanager.cpp: WARNING - MAX_LIGHTS (32) erreicht");
        return nullptr;
    }

    Light* light = new Light;
    light->SetLightType(type);

    if (type == LightType::Point)
        light->SetRadius(100.0f);

    m_lights.push_back(light);
    m_entities.push_back(light);
    Debug::Log("objectmanager.cpp: Light erstellt (Gesamt: ", static_cast<int>(m_lights.size()), ")");
    return light;
}

void ObjectManager::DeleteLight(Light* light)
{
    if (!light) return;

    auto entIt = std::find(m_entities.begin(), m_entities.end(), (Entity*)light);
    if (entIt != m_entities.end())
        m_entities.erase(entIt);

    auto it = std::find(m_lights.begin(), m_lights.end(), light);
    if (it != m_lights.end())
    {
        m_lights.erase(it);
        Memory::SafeDelete(light);
    }
}
