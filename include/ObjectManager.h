#pragma once
#include <vector>
#include "gdxutil.h"
#include "gdxdevice.h"
#include "gdxinterface.h"
#include "Entity.h"
#include "Surface.h"
#include "Mesh.h"
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

    // CREATE
    Camera*   CreateCamera();
    Light*    CreateLight(LightType type);
    Light*    CreateLight(D3DLIGHTTYPE type);
    Shader*   CreateShader();
    Material* CreateMaterial();
    Mesh*     CreateMesh();
    Surface*  CreateSurface();

    void RegisterRenderable(Mesh* mesh);
    void UnregisterRenderable(Mesh* mesh);

    // ADD
    void AddSurfaceToMesh(Mesh* mesh, Surface* surface);
    void AddMeshToMaterial(Material* material, Mesh* mesh);
    void AddMaterialToSurface(Material* material, Surface* surface);
    void AssignShaderToMaterial(Shader* shader, Material* material);
    void AddMaterialToShader(Shader* shader, Material* material);

    // DELETE
    void DeleteSurface(Surface* surface);
    void DeleteMesh(Mesh* mesh);
    void DeleteCamera(Camera* camera);
    void DeleteLight(Light* light);
    void DeleteMaterial(Material* material);
    void DeleteShader(Shader* shader);

    // REMOVE
    void RemoveSurfaceFromMesh(Mesh* mesh, Surface* surface);
    void RemoveMaterialFromShader(Shader* shader, Material* material);
    void MoveSurface(Surface* s, Mesh* from, Mesh* to);

    // GET PREVIOUS
    Surface*  GetPreviousSurface(Surface* currentSurface);
    Mesh*     GetPreviousMesh(Mesh* currentMesh);
    Camera*   GetPreviousCamera(Camera* currentCamera);
    Material* GetPreviousMaterial(Material* currentMaterial);
    Shader*   GetPreviousShader(Shader* currentShader);

    // GET
    void      ProcessMesh();
    Surface*  GetSurface(Mesh* mesh);
    Material* GetStandardMaterial() const;
    Shader*   GetShader(const Surface& surface) const;
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

private:
    std::vector<Entity*>   m_entities;
    std::vector<Surface*>  m_surfaces;
    std::vector<Mesh*>     m_meshes;
    std::vector<Mesh*>     m_renderMeshes;
    std::vector<Camera*>   m_cameras;
    std::vector<Light*>    m_lights;
    std::vector<Material*> m_materials;
    std::vector<Shader*>   m_shaders;
};
