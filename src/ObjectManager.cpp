#include "ObjectManager.h"

#include "Scene.h"
#include "AssetManager.h"
#include "Surface.h"
#include "Mesh.h"
#include "MeshAsset.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "Shader.h"

ObjectManager::ObjectManager(Scene& scene, AssetManager& assets)
    : m_scene(scene), m_assets(assets)
{
}

Camera* ObjectManager::CreateCamera() { return m_scene.CreateCamera(); }
Light* ObjectManager::CreateLight(LightType type) { return m_scene.CreateLight(type); }
Light* ObjectManager::CreateLight(D3DLIGHTTYPE type) { return m_scene.CreateLight(type); }
Shader* ObjectManager::CreateShader() { return m_assets.CreateShader(); }
Material* ObjectManager::CreateMaterial() { return m_assets.CreateMaterial(); }
Surface* ObjectManager::CreateSurface() { return m_assets.CreateSurface(); }
MeshAsset* ObjectManager::CreateMeshAsset() { return m_assets.CreateMeshAsset(); }

Mesh* ObjectManager::CreateMesh()
{
    return m_assets.CreateManagedMesh(m_scene);
}

void ObjectManager::AddSurfaceToMesh(Mesh* mesh, Surface* surface) { m_assets.AddSurfaceToMesh(mesh, surface); }
void ObjectManager::AddMeshToMaterial(Material* material, Mesh* mesh) { m_assets.AddMeshToMaterial(material, mesh); }
void ObjectManager::AssignShaderToMaterial(Shader* shader, Material* material) { m_assets.AssignShaderToMaterial(shader, material); }
void ObjectManager::AddMaterialToShader(Shader* shader, Material* material) { m_assets.AddMaterialToShader(shader, material); }
void ObjectManager::SetSlotMaterial(Mesh* mesh, unsigned int slot, Material* material) { m_assets.SetSlotMaterial(mesh, slot, material); }
bool ObjectManager::SetMeshAsset(Mesh* mesh, MeshAsset* asset, bool deleteOldIfUnused) { return m_assets.SetMeshAsset(m_scene, mesh, asset, deleteOldIfUnused); }
bool ObjectManager::DetachMeshAsset(Mesh* mesh, bool deleteOldIfUnused) { return m_assets.DetachMeshAsset(m_scene, mesh, deleteOldIfUnused); }

void ObjectManager::DeleteSurface(Surface* surface) { m_assets.DeleteSurface(m_scene, surface); }
void ObjectManager::DeleteMesh(Mesh* mesh)
{
    m_assets.DeleteManagedMesh(m_scene, mesh);
}
void ObjectManager::DeleteCamera(Camera* camera) { m_scene.DeleteCamera(camera); }
void ObjectManager::DeleteLight(Light* light) { m_scene.DeleteLight(light); }
void ObjectManager::DeleteMaterial(Material* material) { m_assets.DeleteMaterial(m_scene, material); }
void ObjectManager::DeleteShader(Shader* shader) { m_assets.DeleteShader(shader); }
void ObjectManager::DeleteMeshAsset(MeshAsset* asset) { m_assets.DeleteMeshAsset(m_scene, asset); }

void ObjectManager::RemoveSurfaceFromMesh(Mesh* mesh, Surface* surface) { m_assets.RemoveSurfaceFromMesh(mesh, surface); }
void ObjectManager::RemoveMaterialFromShader(Shader* shader, Material* material) { m_assets.RemoveMaterialFromShader(shader, material); }
void ObjectManager::MoveSurface(Surface* s, Mesh* from, Mesh* to) { m_assets.MoveSurface(s, from, to); }

Surface* ObjectManager::GetPreviousSurface(Surface* currentSurface) { return m_assets.GetPreviousSurface(currentSurface); }
Mesh* ObjectManager::GetPreviousMesh(Mesh* currentMesh) { return m_scene.GetPreviousMesh(currentMesh); }
Camera* ObjectManager::GetPreviousCamera(Camera* currentCamera) { return m_scene.GetPreviousCamera(currentCamera); }
Material* ObjectManager::GetPreviousMaterial(Material* currentMaterial) { return m_assets.GetPreviousMaterial(currentMaterial); }
Shader* ObjectManager::GetPreviousShader(Shader* currentShader) { return m_assets.GetPreviousShader(currentShader); }

void ObjectManager::ProcessMesh()
{
    // Legacy no-op. Mesh preparation now runs through Scene + AssetManager.
}
Surface* ObjectManager::GetSurface(Mesh* mesh) { return m_assets.GetSurface(mesh); }
Surface* ObjectManager::GetSurface(Mesh* mesh, unsigned int index) { return m_assets.GetSurface(mesh, index); }
Material* ObjectManager::GetStandardMaterial() const { return m_assets.GetStandardMaterial(); }
Shader* ObjectManager::GetShader(const Mesh& mesh) const { return m_assets.GetShader(mesh); }
Shader* ObjectManager::GetShader(const Material& material) const { return m_assets.GetShader(material); }

const std::vector<Shader*>& ObjectManager::GetShaders() const { return m_assets.GetShaders(); }
const std::vector<Mesh*>& ObjectManager::GetMeshes() const { return m_scene.GetMeshes(); }
const std::vector<Camera*>& ObjectManager::GetCameras() const { return m_scene.GetCameras(); }
const std::vector<Light*>& ObjectManager::GetLights() const { return m_scene.GetLights(); }
size_t ObjectManager::GetCameraCount() const { return m_scene.GetCameraCount(); }
Camera* ObjectManager::GetCamera(size_t index) const { return m_scene.GetCamera(index); }
size_t ObjectManager::GetLightCount() const { return m_scene.GetLightCount(); }
Light* ObjectManager::GetLight(size_t index) const { return m_scene.GetLight(index); }
void ObjectManager::SetDefaultMaterial(Material* material) { m_assets.SetDefaultMaterial(material); }
