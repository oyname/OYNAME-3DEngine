#pragma once
// ObjectManager: Legacy compatibility shim.
//
// This class contains no logic of its own. Every method delegates
// directly to Scene or AssetManager. New code should call those
// systems directly via GDXEngine::GetScene() and GDXEngine::GetAM().
// ObjectManager exists only to keep existing call sites compiling
// without a forced migration pass.
#include "gdxutil.h"

#include "Light.h"
#include <vector>

class Scene;
class AssetManager;
class Mesh;
class Surface;
class MeshAsset;
class Camera;
class Light;
class Material;
class Shader;

class ObjectManager
{
public:
    ObjectManager(Scene& scene, AssetManager& assets);
    ~ObjectManager() = default;
    void Init() {} // No-op stub kept for call-site compatibility.

    Camera* CreateCamera();
    Light* CreateLight(LightType type);
    Light* CreateLight(D3DLIGHTTYPE type);
    Shader* CreateShader();
    Material* CreateMaterial();
    Mesh* CreateMesh();
    Surface* CreateSurface();
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

    Surface* GetPreviousSurface(Surface* currentSurface);
    Mesh* GetPreviousMesh(Mesh* currentMesh);
    Camera* GetPreviousCamera(Camera* currentCamera);
    Material* GetPreviousMaterial(Material* currentMaterial);
    Shader* GetPreviousShader(Shader* currentShader);

    void ProcessMesh();
    Surface* GetSurface(Mesh* mesh);
    Surface* GetSurface(Mesh* mesh, unsigned int index);
    Material* GetStandardMaterial() const;
    Shader* GetShader(const Mesh& mesh) const;
    Shader* GetShader(const Material& material) const;

    const std::vector<Shader*>& GetShaders() const;
    const std::vector<Mesh*>& GetMeshes() const;
    const std::vector<Camera*>& GetCameras() const;
    const std::vector<Light*>& GetLights() const;

    size_t GetCameraCount() const;
    Camera* GetCamera(size_t index) const;
    size_t GetLightCount() const;
    Light* GetLight(size_t index) const;

    void SetDefaultMaterial(Material* material);
    Scene& GetScene() noexcept { return m_scene; }
    const Scene& GetScene() const noexcept { return m_scene; }
    AssetManager& GetAssetManager() noexcept { return m_assets; }
    const AssetManager& GetAssetManager() const noexcept { return m_assets; }

private:
    Scene& m_scene;
    AssetManager& m_assets;
};
