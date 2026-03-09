#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "Mesh.h"

Scene::~Scene() = default;

static LightType ConvertD3DLightType(D3DLIGHTTYPE d3dType)
{
    return (d3dType == D3DLIGHT_POINT) ? LightType::Point : LightType::Directional;
}

Camera* Scene::CreateCamera()
{
    m_ownedCameras.push_back(std::make_unique<Camera>());
    Camera* camera = m_ownedCameras.back().get();
    m_cameras.push_back(camera);
    return camera;
}

Light* Scene::CreateLight(D3DLIGHTTYPE type)
{
    return CreateLight(ConvertD3DLightType(type));
}

Light* Scene::CreateLight(LightType type)
{
    if (m_lights.size() >= MAX_LIGHTS)
    {
        DBLOG("Scene.cpp: WARNING - MAX_LIGHTS (32) reached");
        return nullptr;
    }

    m_ownedLights.push_back(std::make_unique<Light>());
    Light* light = m_ownedLights.back().get();
    light->SetLightType(type);

    if (type == LightType::Point)
        light->SetRadius(100.0f);

    m_lights.push_back(light);
    DBLOG("Scene.cpp: Light created (total: ", static_cast<int>(m_lights.size()), ")");
    return light;
}

Mesh* Scene::CreateMesh()
{
    m_ownedMeshes.push_back(std::make_unique<Mesh>());
    Mesh* mesh = m_ownedMeshes.back().get();
    m_meshes.push_back(mesh);
    return mesh;
}

void Scene::DeleteMesh(Mesh* mesh)
{
    if (!mesh) return;
    m_meshes.erase(std::remove(m_meshes.begin(), m_meshes.end(), mesh), m_meshes.end());
    RemoveOwned(m_ownedMeshes, mesh);
}

void Scene::DeleteCamera(Camera* camera)
{
    if (!camera) return;
    m_cameras.erase(std::remove(m_cameras.begin(), m_cameras.end(), camera), m_cameras.end());
    RemoveOwned(m_ownedCameras, camera);
}

void Scene::DeleteLight(Light* light)
{
    if (!light) return;
    m_lights.erase(std::remove(m_lights.begin(), m_lights.end(), light), m_lights.end());
    RemoveOwned(m_ownedLights, light);
}

Mesh* Scene::GetPreviousMesh(Mesh* currentMesh)
{
    for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it)
    {
        if (*it == currentMesh)
            return (it != m_meshes.begin()) ? *std::prev(it) : nullptr;
    }
    return nullptr;
}

Camera* Scene::GetPreviousCamera(Camera* currentCamera)
{
    for (auto it = m_cameras.begin(); it != m_cameras.end(); ++it)
    {
        if (*it == currentCamera)
            return (it != m_cameras.begin()) ? *std::prev(it) : nullptr;
    }
    return nullptr;
}
