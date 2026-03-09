#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "gdxutil.h"

#include "Camera.h"
#include "Light.h"
#include "Mesh.h"

#ifndef MAX_LIGHTS
#define MAX_LIGHTS 32
#endif

class Camera;
class Light;
class Mesh;

class Scene
{
public:
    Scene() = default;
    ~Scene();

    void Init() {}

    Camera* CreateCamera();
    Light* CreateLight(LightType type);
    Light* CreateLight(D3DLIGHTTYPE type);
    Mesh* CreateMesh();

    void DeleteMesh(Mesh* mesh);
    void DeleteCamera(Camera* camera);
    void DeleteLight(Light* light);

    Mesh* GetPreviousMesh(Mesh* currentMesh);
    Camera* GetPreviousCamera(Camera* currentCamera);

    const std::vector<Mesh*>& GetMeshes() const noexcept { return m_meshes; }
    const std::vector<Camera*>& GetCameras() const noexcept { return m_cameras; }
    const std::vector<Light*>& GetLights() const noexcept { return m_lights; }

    size_t GetCameraCount() const noexcept { return m_cameras.size(); }
    Camera* GetCamera(size_t index) const noexcept { return (index < m_cameras.size()) ? m_cameras[index] : nullptr; }
    size_t GetLightCount() const noexcept { return m_lights.size(); }
    Light* GetLight(size_t index) const noexcept { return (index < m_lights.size()) ? m_lights[index] : nullptr; }

private:
    template<typename T>
    static bool RemoveOwned(std::vector<std::unique_ptr<T>>& owner, T* ptr)
    {
        auto it = std::find_if(owner.begin(), owner.end(), [ptr](const std::unique_ptr<T>& p) { return p.get() == ptr; });
        if (it == owner.end()) return false;
        owner.erase(it);
        return true;
    }

    std::vector<Mesh*>   m_meshes;
    std::vector<Camera*> m_cameras;
    std::vector<Light*>  m_lights;

    std::vector<std::unique_ptr<Mesh>>   m_ownedMeshes;
    std::vector<std::unique_ptr<Camera>> m_ownedCameras;
    std::vector<std::unique_ptr<Light>>  m_ownedLights;
};

using World = Scene;
