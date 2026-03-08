#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "gdxutil.h"
#include "gdxdevice.h"
#include "gdxinterface.h"
#include "Entity.h"
#include "Surface.h"
#include "Mesh.h"
#include "MeshAsset.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "Shader.h"

#define MAX_LIGHTS 32

class RenderManager;

class ObjectManager
{
public:
    ObjectManager();
    ~ObjectManager();
    void Init() {}

    Camera*    CreateCamera();
    Light*     CreateLight(LightType type);
    Light*     CreateLight(D3DLIGHTTYPE type);
    Shader*    CreateShader();
    Material*  CreateMaterial();
    Mesh*      CreateMesh();
    Surface*   CreateSurface();
    MeshAsset* CreateMeshAsset();

    void AddSurfaceToMesh(Mesh* mesh, Surface* surface);
    void AddMeshToMaterial(Material* material, Mesh* mesh);
    void AssignShaderToMaterial(Shader* shader, Material* material);
    void AddMaterialToShader(Shader* shader, Material* material);

    void SetSlotMaterial(Mesh* mesh, unsigned int slot, Material* material);
    bool SetMeshAsset(Mesh* mesh, MeshAsset* asset, bool deleteOldIfUnused = true);
    bool DetachMeshAsset(Mesh* mesh, bool deleteOldIfUnused = true);

    void DeleteSurface(Surface* surface);
    void DeleteMesh(Mesh* mesh);
    void DeleteCamera(Camera* camera);
    void DeleteLight(Light* light);
    void DeleteMaterial(Material* material);
    void DeleteShader(Shader* shader);
    void DeleteMeshAsset(MeshAsset* asset);

    void RemoveSurfaceFromMesh(Mesh* mesh, Surface* surface);
    void RemoveMaterialFromShader(Shader* shader, Material* material);
    void MoveSurface(Surface* s, Mesh* from, Mesh* to);

    Surface*  GetPreviousSurface(Surface* currentSurface);
    Mesh*     GetPreviousMesh(Mesh* currentMesh);
    Camera*   GetPreviousCamera(Camera* currentCamera);
    Material* GetPreviousMaterial(Material* currentMaterial);
    Shader*   GetPreviousShader(Shader* currentShader);

    void      ProcessMesh();
    Surface*  GetSurface(Mesh* mesh);
    Surface*  GetSurface(Mesh* mesh, unsigned int index);
    Material* GetStandardMaterial() const;
    Shader*   GetShader(const Mesh& mesh) const;
    Shader*   GetShader(const Material& material) const;

    const std::vector<Shader*>& GetShaders() const { return m_shaders; }
    const std::vector<Mesh*>&   GetMeshes()  const { return m_meshes;  }
    const std::vector<Light*>&  GetLights()  const { return m_lights;  }

    size_t GetLightCount() const { return m_lights.size(); }
    Light* GetLight(size_t index) const
    {
        return (index < m_lights.size()) ? m_lights[index] : nullptr;
    }

    void SetDefaultMaterial(Material* material) { m_defaultMaterial = material; }

private:
    bool IsMeshAssetInUse(const MeshAsset* asset) const;
    unsigned int CountMeshAssetUsers(const MeshAsset* asset) const;

    template<typename T>
    static bool RemoveOwned(std::vector<std::unique_ptr<T>>& owner, T* ptr)
    {
        auto it = std::find_if(owner.begin(), owner.end(), [ptr](const std::unique_ptr<T>& p) { return p.get() == ptr; });
        if (it == owner.end()) return false;
        owner.erase(it);
        return true;
    }

    uint32_t m_nextShaderId   = 0;
    uint32_t m_nextMaterialId = 0;

    std::vector<Surface*>   m_surfaces;
    std::vector<MeshAsset*> m_meshAssets;
    std::vector<Mesh*>      m_meshes;
    std::vector<Camera*>    m_cameras;
    std::vector<Light*>     m_lights;
    std::vector<Material*>  m_materials;
    std::vector<Shader*>    m_shaders;

    std::vector<std::unique_ptr<Surface>>   m_ownedSurfaces;
    std::vector<std::unique_ptr<MeshAsset>> m_ownedMeshAssets;
    std::vector<std::unique_ptr<Mesh>>      m_ownedMeshes;
    std::vector<std::unique_ptr<Camera>>    m_ownedCameras;
    std::vector<std::unique_ptr<Light>>     m_ownedLights;
    std::vector<std::unique_ptr<Material>>  m_ownedMaterials;
    std::vector<std::unique_ptr<Shader>>    m_ownedShaders;

    Material* m_defaultMaterial = nullptr;
};
