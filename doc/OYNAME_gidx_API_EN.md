# OYNAME-3DEngine — `gidx.h` API Reference and Application Development

**Version:** February 2026

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Type Aliases](#2-type-aliases)
3. [Application Structure](#3-application-structure)
4. [Graphics Device and Display](#4-graphics-device-and-display)
5. [Camera](#5-camera)
6. [Lighting](#6-lighting)
7. [Mesh and Surface](#7-mesh-and-surface)
8. [Vertex Building](#8-vertex-building)
9. [GPU Upload](#9-gpu-upload)
10. [Material](#10-material)
11. [Texture](#11-texture)
12. [Shader](#12-shader)
13. [Entity Transform](#13-entity-transform)
14. [Entity Properties](#14-entity-properties)
15. [Scene Hierarchy](#15-scene-hierarchy)
16. [Render Layers and Camera Culling](#16-render-layers-and-camera-culling)
17. [Render-to-Texture](#17-render-to-texture)
18. [Skeletal Animation](#18-skeletal-animation)
19. [Collision](#19-collision)
20. [Render Loop](#20-render-loop)
21. [Debug Utilities](#21-debug-utilities)
22. [Complete Examples](#22-complete-examples)

---

## 1. Introduction

`gidx.h` is the only header game code needs to include. It exposes the entire engine API through the `Engine::` namespace as thin inline wrappers. No DirectX types, no COM interfaces, and no internal engine classes are visible to game code.

All objects are created and destroyed through the engine API. Never use `new` or `delete` on engine objects.

```cpp
#include "gidx.h"

int main()
{
    Engine::Graphics(1280, 720);
    // ... setup and game loop
    return 0;
}
```

---

## 2. Type Aliases

| Alias | Underlying Type | Description |
|---|---|---|
| `LPENTITY` | `Entity*` | Any scene object (mesh, camera, light) |
| `LPMATERIAL` | `Material*` | Material instance |
| `LPSHADER` | `Shader*` | Compiled shader pair |
| `LPSURFACE` | `Surface*` | Geometry surface (sub-mesh) |
| `LPTEXTURE` | `Texture*` | Loaded or created texture |
| `LPRENDERTARGET` | `RenderTextureTarget*` | Render-to-texture target |
| `TEXTURE` | `Texture` | Value type alias |
| `SURFACE` | `Surface` | Value type alias |
| `SHADER` | `Shader` | Value type alias |

---

## 3. Application Structure

### Minimum Application

```cpp
#include "gidx.h"

int main()
{
    Engine::Graphics(1280, 720);

    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 0.0f, -5.0f);

    while (Windows::MainLoop())
    {
        Core::BeginFrame();
        Engine::Cls(0, 0, 0);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();
        Core::EndFrame();
    }
    return 0;
}
```

### Frame Loop Functions

```cpp
Core::BeginFrame();     // Advances timer, frame counter
Engine::Cls(r, g, b);  // Clear backbuffer (0–255 integer values)
Engine::UpdateWorld();  // Update all entity transforms and GPU buffers
Engine::RenderWorld();  // Execute shadow pass + render pass
Engine::Flip();         // Present backbuffer to screen
Core::EndFrame();       // Frame statistics
```

`Engine::Cls` accepts integer color values in the range 0–255:

```cpp
Engine::Cls(0, 64, 128);        // Dark blue
Engine::Cls(0, 0, 0);           // Black
Engine::Cls(0, 64, 128, 255);   // With explicit alpha
```

---

## 4. Graphics Device and Display

### Initialize Graphics

```cpp
unsigned int Graphics(unsigned int width, unsigned int height, bool windowed = true);
```

Sets the rendering resolution. Call once at startup, after `Core::CreateEngine()`.

```cpp
Engine::Graphics(1280, 720);           // Windowed
Engine::Graphics(1920, 1080, false);   // Fullscreen
```

### Display Query

```cpp
unsigned int CountGfxDrivers();
std::string  GfxDriverName();
void         SetGfxDriver(unsigned int adapter);
unsigned int CountOutputs();
void         SetOutput(unsigned int output);
unsigned int CountGfxModes(unsigned int output);
unsigned int GfxModeWidth(unsigned int mode);
unsigned int GfxModeHeight(unsigned int mode);
unsigned int GetGfxModeFrequency(unsigned int mode);
unsigned int GfxModeDepth();
bool         GfxModeExists(int width, int height, int frequency);
unsigned int GfxColorDepth();
unsigned int GetWidth();
unsigned int GetHeight();
unsigned int GfxGetDirectXVersion();
unsigned int GetCurrentAdapter();
int          GetMaxFrequency(unsigned int width, unsigned int height);
bool         GfxFormatSupported(GXFORMAT format);
```

Typical adapter selection loop:

```cpp
for (unsigned int i = 0; i < Engine::CountGfxDrivers(); ++i)
{
    Engine::SetGfxDriver(i);
    // Engine::GfxDriverName() returns name of adapter i
}
```

### VSync

```cpp
void SetVSync(int interval);   // 1 = on, 0 = off
int  GetVSync();
```

---

## 5. Camera

### Create Camera

```cpp
void CreateCamera(LPENTITY* camera);
```

Creates a perspective camera with a 60° FOV, aspect ratio matching the current window, near plane 0.1, far plane 1000. Sets itself as the active camera automatically.

```cpp
LPENTITY camera = nullptr;
Engine::CreateCamera(&camera);
Engine::PositionEntity(camera, 0.0f, 2.0f, -10.0f);
```

### Set Active Camera

```cpp
void SetCamera(LPENTITY camera);
```

Switches which camera the next `RenderWorld()` uses for the main scene pass.

### Projection Mode

```cpp
void SetCameraPersp(LPENTITY camera,
    float fovDegrees = 60.0f,
    float nearZ = 0.1f,
    float farZ  = 1000.0f);

void SetCameraOrtho(LPENTITY camera,
    float width, float height,
    float nearZ = 0.1f,
    float farZ  = 1000.0f);
```

`SetCameraPersp` recalculates the projection matrix using the current window aspect ratio.

### Synchronize Light to Camera

```cpp
void PositionLightAtCamera(Light* light, Camera* camera,
    DirectX::XMVECTOR offset = DirectX::XMVectorZero());
```

Copies the camera's position and orientation to a light, optionally adding an offset. Useful for flashlight effects.

---

## 6. Lighting

### Create Light

```cpp
void CreateLight(LPENTITY* light, D3DLIGHTTYPE type);
```

Light types:

| Constant | Value | Meaning |
|---|---|---|
| `D3DLIGHT_POINT` | 1 | Omnidirectional point light |
| `D3DLIGHT_SPOT` | 2 | Cone spotlight |
| `D3DLIGHT_DIRECTIONAL` | (not shown, added) | Directional light |

```cpp
LPENTITY sunLight = nullptr;
Engine::CreateLight(&sunLight, D3DLIGHT_DIRECTIONAL);
Engine::TurnEntity(sunLight, 45.0f, 0.0f, 0.0f);
Engine::LightColor(sunLight, 1.0f, 1.0f, 1.0f);
```

### Light Color

```cpp
void LightColor(LPENTITY light, float r, float g, float b, float a = 1.0f);
```

Values are in the range 0.0–1.0. Values above 1.0 increase intensity beyond standard white.

### Global Ambient

```cpp
void SetAmbientColor(float r, float g, float b, float a = 1.0f);
```

Sets the scene-wide ambient light that affects all materials.

### Directional Shadow Light

```cpp
void SetDirectionalLight(LPENTITY light);
```

Registers a light as the single directional shadow-casting light. Only one light can cast shadows. Must be called before shadows appear.

### Shadow Map Configuration

```cpp
void LightShadowOrthoSize(LPENTITY light, float size);
void LightShadowPlanes(LPENTITY light, float nearPlane, float farPlane);
void LightShadowFov(LPENTITY light, float fovRadians);
```

`LightShadowOrthoSize` sets the orthographic projection size in world units. Smaller values produce sharper shadows but cover less of the scene. `LightShadowPlanes` sets the near and far clip planes of the shadow camera — a tighter range reduces shadow acne. `LightShadowFov` is for perspective (spotlight) shadow cameras.

For a typical indoor scene spanning about 15 world units:

```cpp
Engine::LightShadowOrthoSize(sunLight, 15.0f);
Engine::LightShadowPlanes(sunLight, 0.5f, 25.0f);
```

### Shadow and Lighting Per Object

```cpp
void EntityCastShadows(LPENTITY entity, bool enabled);
void MaterialReceiveShadows(LPMATERIAL material, bool enabled);
```

---

## 7. Mesh and Surface

### Create Mesh

```cpp
void CreateMesh(LPENTITY* mesh);
```

Allocates a mesh entity and its GPU constant buffer. The mesh has no geometry until surfaces are added.

### Create Surface

```cpp
void CreateSurface(LPSURFACE* surface, LPENTITY entity);
void CreateSurface(LPSURFACE* surface, LPENTITY entity, unsigned int* outSlot);
```

Adds a new surface (sub-mesh) to the mesh. `outSlot` receives the slot index assigned to the surface, which is used for per-slot material assignment.

```cpp
LPENTITY cube = nullptr;
LPSURFACE surf = nullptr;
unsigned int slot = 0;

Engine::CreateMesh(&cube);
Engine::CreateSurface(&surf, cube, &slot);
// ... add vertex data ...
Engine::FillBuffer(surf);
```

### Surface Queries

```cpp
LPSURFACE   GetSurface(LPENTITY entity);                            // First surface
LPSURFACE   GetSurface(LPENTITY entity, unsigned int index);        // By slot index
LPSURFACE   EntitySurface(LPENTITY entity, unsigned int index = 0); // Alias
unsigned int GetSurfaceCount(LPENTITY entity);
unsigned int GetSlotCount(LPENTITY entity);                         // Alias
bool         HasSlot(LPENTITY entity, unsigned int slot);
bool         GetSurfaceSlot(LPENTITY entity, LPSURFACE surface, unsigned int* outSlot);
```

### Asset Sharing

Multiple mesh entities can share the same geometry asset, each with different transforms and materials:

```cpp
LPENTITY original = nullptr;
LPENTITY copy     = nullptr;

Engine::CreateMesh(&original);
// ... build geometry on original ...

Engine::CreateMesh(&copy);
Engine::ShareMeshAsset(original, copy);          // copy now shares original's geometry

Engine::SetSlotMaterial(copy, 0, differentMat);  // each can have its own materials
```

### Wireframe

```cpp
void SurfaceWireframe(LPSURFACE surface, bool enabled);
bool SurfaceWireframe(LPSURFACE surface);
```

---

## 8. Vertex Building

All vertex data is accumulated into the surface before calling `FillBuffer`. The order is: for each vertex, call `AddVertex` followed by the optional attribute functions in any order.

### Add Vertex

```cpp
void AddVertex(LPSURFACE surface, float x, float y, float z);
void AddVertex(LPSURFACE surface, DirectX::XMVECTOR vec);
void AddVertex(LPSURFACE surface, DirectX::XMFLOAT3 vec);
void AddVertex(int index, LPSURFACE surface, DirectX::XMVECTOR vec);
void AddVertex(int index, LPSURFACE surface, DirectX::XMFLOAT3 vec);
```

The `index` overloads update an existing vertex by index instead of appending.

### Vertex Attributes

```cpp
void VertexNormal(LPSURFACE surface, float x, float y, float z);
void VertexNormal(LPSURFACE surface, unsigned int index, float x, float y, float z);

void VertexColor(LPSURFACE surface, unsigned int r, unsigned int g, unsigned int b);
void VertexColor(LPSURFACE surface, unsigned int index, unsigned int r, unsigned int g, unsigned int b);

void VertexTexCoord(LPSURFACE surface, float u, float v);    // UV channel 1
void VertexTexCoord2(LPSURFACE surface, float u, float v);   // UV channel 2 (lightmap / detail)
```

Color values are in the range 0–255 and are converted to 0.0–1.0 internally.

### Add Triangle

```cpp
void AddTriangle(LPSURFACE surface, unsigned int a, unsigned int b, unsigned int c);
```

Adds three vertex indices forming one triangle.

### Building a Triangle

```cpp
LPSURFACE surf = nullptr;
Engine::CreateSurface(&surf, mesh);

Engine::AddVertex(surf, -1.0f, 0.0f, 0.0f);
Engine::VertexNormal(surf, 0.0f, 1.0f, 0.0f);
Engine::VertexTexCoord(surf, 0.0f, 0.0f);

Engine::AddVertex(surf,  1.0f, 0.0f, 0.0f);
Engine::VertexNormal(surf, 0.0f, 1.0f, 0.0f);
Engine::VertexTexCoord(surf, 1.0f, 0.0f);

Engine::AddVertex(surf,  0.0f, 2.0f, 0.0f);
Engine::VertexNormal(surf, 0.0f, 1.0f, 0.0f);
Engine::VertexTexCoord(surf, 0.5f, 1.0f);

Engine::AddTriangle(surf, 0, 1, 2);
Engine::FillBuffer(surf);
```

---

## 9. GPU Upload

### Upload to GPU

```cpp
void FillBuffer(LPSURFACE surface);
void FillBuffer(LPENTITY entity, unsigned int slot);
```

Uploads all CPU-side vertex and index data to GPU buffers. Must be called after all vertex data has been set and before the mesh appears on screen. Tangent vectors are computed automatically if the shader requires them.

### Dynamic Updates

After the initial `FillBuffer`, individual streams can be updated per-frame for dynamic geometry:

```cpp
void UpdateVertexBuffer(LPSURFACE surface);   // Positions
void UpdateNormalBuffer(LPSURFACE surface);   // Normals
void UpdateColorBuffer(LPSURFACE surface);    // Colors
```

Call the appropriate update function after modifying the CPU-side data. The GPU buffer must have been created as `DYNAMIC` — this is handled automatically by `BufferManager` for dynamically updated surfaces.

---

## 10. Material

### Create Material

```cpp
void CreateMaterial(LPMATERIAL* material, SHADER* shader = nullptr);
```

Creates a material and associates it with a shader. If `shader` is `nullptr`, the engine's default standard shader is used.

```cpp
LPMATERIAL mat = nullptr;
Engine::CreateMaterial(&mat);
```

### Create Skinned Material

```cpp
void CreateSkinnedMaterial(LPMATERIAL* material);
```

Creates a material using the built-in skinning shader (`ShaderKey::StandardSkinned`). Use this for meshes driven by `SetEntityBoneMatrices`.

### Assign Material to Mesh

```cpp
void EntityMaterial(LPENTITY entity, LPMATERIAL material);
void SetSlotMaterial(LPENTITY entity, unsigned int slot, LPMATERIAL material);
bool SetSurfaceMaterial(LPENTITY entity, LPSURFACE surface, LPMATERIAL material);
```

`EntityMaterial` assigns the material to the mesh's primary (slot 0) surface. `SetSlotMaterial` targets a specific slot by index. `SetSurfaceMaterial` looks up the slot index for a given surface pointer and assigns the material to that slot.

### Base Color

```cpp
void MaterialColor(LPMATERIAL material, float r, float g, float b, float a = 1.0f);
```

Sets the base diffuse / albedo color. Values are 0.0–1.0.

### PBR Mode

```cpp
void MaterialUsePBR(LPMATERIAL material, bool enabled);
void MaterialMetallic(LPMATERIAL material, float metallic);
void MaterialRoughness(LPMATERIAL material, float roughness);
void MaterialNormalScale(LPMATERIAL material, float scale);
void MaterialOcclusionStrength(LPMATERIAL material, float strength);
void MaterialEmissiveColor(LPMATERIAL material, float r, float g, float b, float intensity = 1.0f);
void MaterialShininess(LPMATERIAL material, float shininess);
```

PBR mode is opt-in. The default shading path is legacy Blinn-Phong, preserving backward compatibility. Enable PBR explicitly:

```cpp
Engine::MaterialUsePBR(mat, true);
Engine::MaterialMetallic(mat, 0.0f);
Engine::MaterialRoughness(mat, 0.5f);
```

### UV Tiling and Offset

```cpp
void MaterialUVTilingOffset(LPMATERIAL material,
    float tileU, float tileV,
    float offU = 0.0f, float offV = 0.0f);
```

### Texture Blend Mode (Secondary Texture)

```cpp
void MaterialBlendMode(LPMATERIAL material, int mode = 0);
void MaterialBlendFactor(LPMATERIAL material, float factor);
```

Blend modes for the secondary texture (decal / detail map):

| Mode | Name | Typical Use |
|---|---|---|
| 0 | Off | Single texture only |
| 1 | Multiply | Lightmap, shadow darkening |
| 2 | Multiply ×2 | Detail map, brightness correction |
| 3 | Additive | Glow, fire, light |
| 4 | Lerp (alpha) | Decal, sticker — uses alpha of texture 2 |
| 5 | Luminance | Overlay with black background |

`MaterialBlendFactor` controls the blend strength (0.0–1.0).

### Transparency

```cpp
void MaterialTransparent(LPMATERIAL material, bool enabled);
void MaterialAlphaCutoff(LPMATERIAL material, float cutoff);
void MaterialAlphaTest(LPMATERIAL material, bool enabled);
```

`MaterialTransparent` enables full alpha blending (SRC_ALPHA / INV_SRC_ALPHA) and routes the material into the back-to-front transparent pass. `MaterialAlphaCutoff` enables alpha test discarding — pixels below the cutoff value are discarded. Alpha test is cheaper than full blending and suitable for foliage and similar geometry.

### Shadow Interaction

```cpp
void MaterialReceiveShadows(LPMATERIAL material, bool enabled);
```

---

## 11. Texture

### Load from File

```cpp
void LoadTexture(LPLPTEXTURE texture, const wchar_t* filename);
```

```cpp
LPTEXTURE albedo = nullptr;
Engine::LoadTexture(&albedo, L"..\\media\\brick_albedo.png");
```

Supported formats depend on the underlying WIC loader (BMP, PNG, JPG, TGA, DDS, and others).

### Assign to Material

The preferred API uses named semantic functions:

```cpp
void MaterialSetAlbedo(LPMATERIAL material, LPTEXTURE texture);    // Slot t0
void MaterialSetNormal(LPMATERIAL material, LPTEXTURE texture);    // Slot t1
void MaterialSetORM(LPMATERIAL material, LPTEXTURE texture);       // Slot t2
void MaterialSetDecal(LPMATERIAL material, LPTEXTURE texture);     // Slot t3
void MaterialSetOcclusion(LPMATERIAL material, LPTEXTURE texture); // Slot t4
void MaterialSetRoughness(LPMATERIAL material, LPTEXTURE texture); // Slot t5
void MaterialSetMetallic(LPMATERIAL material, LPTEXTURE texture);  // Slot t6
```

`MaterialSetNormal` automatically activates the `MF_USE_NORMAL_MAP` flag. `MaterialSetORM` automatically activates the `MF_USE_ORM_MAP` flag.

The legacy slot API is also available for backward compatibility:

```cpp
void MaterialTexture(LPMATERIAL material, LPTEXTURE texture, int slot = 0);
```

Slot mapping: 0 = Albedo, 1 = Normal, 2 = ORM, 3 = Decal.

### Assign to Entity (convenience)

```cpp
void EntityTexture(LPENTITY entity, LPTEXTURE texture);
```

Assigns the texture as the albedo of the material in slot 0 of the entity.

### Create Dynamic Texture

```cpp
HRESULT CreateTexture(LPLPTEXTURE texture, int width, int height);
HRESULT LockBuffer(LPTEXTURE texture);
void    UnlockBuffer(LPTEXTURE texture);
void    SetPixel(LPTEXTURE texture, int x, int y,
                 unsigned char r, unsigned char g,
                 unsigned char b, unsigned char alpha);
Color   GetColor(LPTEXTURE texture, int x, int y);
```

```cpp
LPTEXTURE dynTex = nullptr;
Engine::CreateTexture(&dynTex, 256, 256);

Engine::LockBuffer(dynTex);
for (int y = 0; y < 256; ++y)
    for (int x = 0; x < 256; ++x)
        Engine::SetPixel(dynTex, x, y, (unsigned char)x, (unsigned char)y, 128, 255);
Engine::UnlockBuffer(dynTex);

Engine::MaterialSetAlbedo(mat, dynTex);
```

---

## 12. Shader

### Create Custom Shader

```cpp
HRESULT CreateShader(LPSHADER* shader,
    const std::wstring& vertexShaderFile,
    const std::string&  vertexEntryPoint,
    const std::wstring& pixelShaderFile,
    const std::string&  pixelEntryPoint,
    DWORD flags);
```

`flags` is a combination of `D3DVERTEX_FLAGS` values describing the vertex format.

```cpp
LPSHADER myShader = nullptr;
DWORD flags = Engine::CreateVertexFlags(
    true,  // position
    true,  // normal
    false, // color
    true,  // texcoord1
    false, // texcoord2
    true   // tangent
);

HRESULT hr = Engine::CreateShader(&myShader,
    L"..\\..\\shaders\\MyVertex.hlsl",   "VSMain",
    L"..\\..\\shaders\\MyPixel.hlsl",    "PSMain",
    flags);
```

### Vertex Flag Helper

```cpp
DWORD CreateVertexFlags(
    bool hasPosition  = true,
    bool hasNormal    = false,
    bool hasColor     = false,
    bool hasTexCoord1 = false,
    bool hasTexCoord2 = false,
    bool hasTangent   = false);
```

---

## 13. Entity Transform

All transform functions work on any entity type (Mesh, Camera, Light).

### Position

```cpp
void PositionEntity(LPENTITY entity, float x, float y, float z);
void PositionEntity(LPENTITY entity, DirectX::XMVECTOR pos);
```

Sets the entity's local position to an absolute value.

### Move

```cpp
void MoveEntity(LPENTITY entity, float x, float y, float z,
                Space mode = Space::Local);
```

Translates the entity by a delta. `Space::Local` moves along the entity's own axes; `Space::World` moves along world axes.

### Rotate

```cpp
void RotateEntity(LPENTITY entity, float rotX, float rotY, float rotZ,
                  Space mode = Space::Local);
void RotateEntity(LPENTITY entity, DirectX::XMVECTOR quaternion);
```

Sets the entity's rotation to an absolute orientation. Euler angles are in degrees.

### Turn

```cpp
void TurnEntity(LPENTITY entity, float rotX, float rotY, float rotZ,
                Space mode = Space::Local);
```

Applies a rotation delta (incremental rotation), unlike `RotateEntity` which sets an absolute orientation.

### Scale

```cpp
void ScaleEntity(LPENTITY entity, float x, float y, float z);
```

### Look At

```cpp
void LookAt(LPENTITY entity, float targetX, float targetY, float targetZ);
void LookAt(LPENTITY entity, DirectX::XMVECTOR target);
```

Orients the entity so that its forward axis (+Z) points toward the target position.

### Position Getters

```cpp
DirectX::XMVECTOR EntityPosition(LPENTITY entity);   // World position (with parent)
float EntityX(LPENTITY entity);
float EntityY(LPENTITY entity);
float EntityZ(LPENTITY entity);

DirectX::XMVECTOR EntityLocalPosition(LPENTITY entity); // Local position
float EntityLocalX(LPENTITY entity);
float EntityLocalY(LPENTITY entity);
float EntityLocalZ(LPENTITY entity);
```

`EntityPosition` returns the world-space position including all parent transforms. `EntityLocalPosition` returns the value stored in the local transform.

---

## 14. Entity Properties

### Active and Visible

```cpp
void EntityActive(LPENTITY entity, bool active);
bool EntityActive(LPENTITY entity);

void ShowEntity(LPENTITY entity, bool visible);
bool EntityVisible(LPENTITY entity);
```

`EntityActive(false)` completely disables the entity — no update, no physics, no rendering. `ShowEntity(false)` keeps the entity updating and simulating but removes it from rendering.

### Shadows

```cpp
void EntityCastShadows(LPENTITY entity, bool enabled);
```

Controls whether the entity appears in the shadow pass. Disabling reduces shadow map draw calls.

---

## 15. Scene Hierarchy

```cpp
void    SetEntityParent(LPENTITY child, LPENTITY parent);
void    DetachEntity(LPENTITY entity);
LPENTITY GetEntityParent(LPENTITY entity);
```

When a parent is set, the child's local transform is preserved. The world matrix is computed as:

```
worldMatrix = localMatrix * parent->GetWorldMatrix()
```

Parent relationships are hierarchical and can be nested to any depth. Setting `parent = nullptr` is equivalent to calling `DetachEntity`.

```cpp
// Car body with attached wheel
LPENTITY car   = nullptr;
LPENTITY wheel = nullptr;

Engine::CreateMesh(&car);
Engine::CreateMesh(&wheel);

Engine::SetEntityParent(wheel, car);           // wheel follows car
Engine::PositionEntity(wheel, 1.5f, 0.0f, 0.0f); // offset in car's local space

// During update: rotating car also moves the wheel
Engine::TurnEntity(car, 0.0f, Timer::GetDeltaTime() * 90.0f, 0.0f);
```

---

## 16. Render Layers and Camera Culling

### Entity Layers

```cpp
void     EntityLayer(LPENTITY entity, uint32_t layerMask);
uint32_t EntityLayer(LPENTITY entity);
```

### Camera Cull Mask

```cpp
void     CameraCullMask(LPENTITY camera, uint32_t mask);
uint32_t CameraCullMask(LPENTITY camera);
```

An entity is visible to a camera only when `(entity->layerMask & camera->cullMask) != 0`.

Available layer constants (`RenderLayers.h`):

```cpp
LAYER_DEFAULT    // (1 << 0)  Standard objects
LAYER_UI         // (1 << 1)  HUD / UI
LAYER_REFLECTION // (1 << 2)  Reflection pass
LAYER_SHADOW     // (1 << 3)  Shadow-only
LAYER_FX         // (1 << 4)  Particles and effects
LAYER_5          // (1 << 5)
LAYER_6          // (1 << 6)
LAYER_7          // (1 << 7)
LAYER_ALL        // 0xFFFFFFFF  Everything
```

Render a UI mesh only through a dedicated UI camera:

```cpp
Engine::EntityLayer(uiPanel, LAYER_UI);
Engine::CameraCullMask(mainCamera, LAYER_DEFAULT | LAYER_FX);  // not UI
Engine::CameraCullMask(uiCamera, LAYER_UI);
```

---

## 17. Render-to-Texture

### Create and Release

```cpp
void CreateRenderTexture(LPRENDERTARGET* rtt, UINT width, UINT height);
void ReleaseRenderTexture(LPRENDERTARGET* rtt);
```

### Configure

```cpp
void SetRTTClearColor(LPRENDERTARGET rtt, float r, float g, float b, float a = 1.0f);
```

### Activate and Restore

```cpp
void SetRenderTarget(LPRENDERTARGET rtt, LPENTITY rttCamera = nullptr);
void ResetRenderTarget();
```

### Use as Texture

```cpp
LPTEXTURE GetRTTTexture(LPRENDERTARGET rtt);
```

### Complete RTT Example

```cpp
LPRENDERTARGET mirrorRTT = nullptr;
Engine::CreateRenderTexture(&mirrorRTT, 512, 512);
Engine::SetRTTClearColor(mirrorRTT, 0.1f, 0.1f, 0.1f);

// Create mirror material
LPMATERIAL mirrorMat = nullptr;
Engine::CreateMaterial(&mirrorMat);

// In the render loop:
Engine::SetRenderTarget(mirrorRTT, reflectionCamera);
Engine::RenderWorld();                             // renders into texture

Engine::ResetRenderTarget();                       // back to backbuffer
Engine::MaterialSetAlbedo(mirrorMat, Engine::GetRTTTexture(mirrorRTT));

Engine::Cls(0, 0, 0);
Engine::RenderWorld();                             // renders main scene
Engine::Flip();
```

---

## 18. Skeletal Animation

### Setup Bone Data

Before uploading to the GPU, assign bone indices and weights to each vertex:

```cpp
void VertexBoneData(LPSURFACE surface, unsigned int vertexIndex,
    unsigned int b0, unsigned int b1, unsigned int b2, unsigned int b3,
    float        w0, float        w1, float        w2, float        w3);
```

The four weights should sum to 1.0. Vertices with fewer than four bone influences should set unused indices to 0 and unused weights to 0.0.

### Upload Bone Transforms

```cpp
void SetEntityBoneMatrices(LPENTITY entity,
    const DirectX::XMMATRIX* matrices,
    uint32_t count);
```

Call every frame with the current pose. `count` must not exceed 128 (`MAX_BONES`). On the first call, the bone constant buffer is created on the mesh. Subsequent calls update it.

### Complete Skinned Mesh Setup

```cpp
LPENTITY arm = nullptr;
LPSURFACE surf = nullptr;
LPMATERIAL skinMat = nullptr;

Engine::CreateMesh(&arm);
Engine::CreateSurface(&surf, arm);
Engine::CreateSkinnedMaterial(&skinMat);
Engine::SetSlotMaterial(arm, 0, skinMat);

// For each vertex:
Engine::AddVertex(surf, x, y, z);
Engine::VertexNormal(surf, nx, ny, nz);
Engine::VertexTexCoord(surf, u, v);
Engine::VertexBoneData(surf, vertexIndex, bone0, bone1, 0, 0, w0, w1, 0.0f, 0.0f);

Engine::FillBuffer(surf);

// Per frame:
DirectX::XMMATRIX poseMatrices[2];
poseMatrices[0] = /* shoulder transform */;
poseMatrices[1] = /* elbow transform */;
Engine::SetEntityBoneMatrices(arm, poseMatrices, 2);
```

---

## 19. Collision

### Set Collision Mode

```cpp
void EntityCollisionMode(LPENTITY entity, COLLISION mode);
```

```cpp
enum COLLISION { NONE = 0, BOX = 1, SPHERE = 2 };
```

`BOX` enables OBB (oriented bounding box) collision testing. `SPHERE` enables sphere collision testing.

### Test Collision

```cpp
bool EntityCollision(LPENTITY entity1, LPENTITY entity2);
```

Returns `true` if the two entities' collision volumes overlap. Both entities must have a compatible collision mode set.

### Get OBB

```cpp
DirectX::BoundingOrientedBox* EntityOBB(LPENTITY entity);
```

Returns a pointer to the entity's oriented bounding box for custom intersection tests.

---

## 20. Render Loop

### Complete Render Loop Structure

```cpp
while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
{
    Core::BeginFrame();

    float dt = (float)Timer::GetDeltaTime();

    // Game logic
    Engine::TurnEntity(cube, 0.0f, dt * 60.0f, 0.0f);

    // Rendering
    Engine::Cls(0, 0, 0);
    Engine::UpdateWorld();
    Engine::RenderWorld();
    Engine::Flip();

    Core::EndFrame();
}
```

### Timer

```cpp
Timer::GetDeltaTime()    // double — seconds since last frame
Timer::GetFPS()          // double — current FPS
Timer::GetFixedStep()    // double — fixed step duration (1/60 s)
Timer::GetFixedSteps()   // int    — fixed steps to process this frame
```

Always use `Timer::GetDeltaTime()` for frame-rate independent movement. Never use `Time.DeltaTime()`.

---

## 21. Debug Utilities

### Print Single Mesh

```cpp
void DebugPrintMesh(LPENTITY entity);
```

Prints a tree view of the mesh's internal state to the debug console, including the MeshAsset pointer, all slots and surfaces with vertex and index counts, and the resolved material for each slot.

```
LPENTITY (0x00AABBCC)
+-- Mesh : Entity  active=1  visible=1  layer=1
      +-- transform   pos=(0.00, 1.00, 5.00)
      +-- meshRenderer
            +-- asset --> MeshAsset (0x00DDEEFF)  slots=2  active=2
            |     +-- m_slots[0] --> Surface (0x...)  verts=24  idx=36  active=1
            |     +-- m_slots[1] --> Surface (0x...)  verts=16  idx=24  active=1
            +-- resolved materials per slot
                  +-- slot[0] --> Material (0x...)  [slotMaterials]
                  +-- slot[1] --> Material (0x...)  [engineDefaultMaterial]
```

### Print Entire Scene

```cpp
void DebugPrintScene();
```

Calls `DebugPrintMesh` for every mesh registered in the `ObjectManager`.

### Debug Output Convention

All `Debug::Log` calls must begin with the source filename:

```cpp
Debug::Log("myfile.cpp: object created successfully");
Debug::LogError("myfile.cpp: failed to allocate buffer");
Debug::LogWarning("myfile.cpp: unexpected state, continuing");
Debug::LogHr(__FILE__, __LINE__, hr);
```

---

## 22. Complete Examples

### Example 1 — Minimal PBR Mesh

```cpp
#include "gidx.h"
#include "geometry.h"  // CreateCube()

int main()
{
    Engine::Graphics(1280, 720);

    // Camera
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 2.0f, -8.0f);
    Engine::LookAt(camera, 0.0f, 0.0f, 0.0f);

    // Directional light with shadow
    LPENTITY sunLight = nullptr;
    Engine::CreateLight(&sunLight, D3DLIGHT_DIRECTIONAL);
    Engine::TurnEntity(sunLight, 45.0f, -30.0f, 0.0f);
    Engine::LightColor(sunLight, 1.2f, 1.1f, 1.0f);
    Engine::SetDirectionalLight(sunLight);
    Engine::SetAmbientColor(0.15f, 0.15f, 0.2f);
    Engine::LightShadowOrthoSize(sunLight, 20.0f);
    Engine::LightShadowPlanes(sunLight, 1.0f, 40.0f);

    // Textures
    LPTEXTURE albedoTex = nullptr;
    LPTEXTURE normalTex = nullptr;
    LPTEXTURE ormTex    = nullptr;
    Engine::LoadTexture(&albedoTex, L"..\\media\\stone_albedo.png");
    Engine::LoadTexture(&normalTex, L"..\\media\\stone_normal.png");
    Engine::LoadTexture(&ormTex,    L"..\\media\\stone_orm.png");

    // Material
    LPMATERIAL mat = nullptr;
    Engine::CreateMaterial(&mat);
    Engine::MaterialUsePBR(mat, true);
    Engine::MaterialSetAlbedo(mat, albedoTex);
    Engine::MaterialSetNormal(mat, normalTex);
    Engine::MaterialSetORM(mat, ormTex);
    Engine::MaterialRoughness(mat, 0.7f);
    Engine::MaterialMetallic(mat, 0.0f);
    Engine::MaterialReceiveShadows(mat, true);

    // Mesh
    LPENTITY cube = nullptr;
    CreateCube(&cube, mat);
    Engine::PositionEntity(cube, 0.0f, 0.0f, 0.0f);
    Engine::EntityCastShadows(cube, true);

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();

        float dt = (float)Timer::GetDeltaTime();
        Engine::TurnEntity(cube, 0.0f, dt * 45.0f, 0.0f);

        Engine::Cls(10, 10, 15);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }
    return 0;
}
```

### Example 2 — Multi-Surface Mesh with Per-Slot Materials

```cpp
LPENTITY vehicle = nullptr;
Engine::CreateMesh(&vehicle);

// Surface 0: body
LPSURFACE bodyS = nullptr;
unsigned int bodySlot = 0;
Engine::CreateSurface(&bodyS, vehicle, &bodySlot);
// ... body vertex data ...
Engine::FillBuffer(bodyS);

// Surface 1: glass
LPSURFACE glassS = nullptr;
unsigned int glassSlot = 0;
Engine::CreateSurface(&glassS, vehicle, &glassSlot);
// ... glass vertex data ...
Engine::FillBuffer(glassS);

// Per-slot materials
LPMATERIAL bodyMat  = nullptr;
LPMATERIAL glassMat = nullptr;
Engine::CreateMaterial(&bodyMat);
Engine::CreateMaterial(&glassMat);
Engine::MaterialTransparent(glassMat, true);
Engine::MaterialColor(glassMat, 0.8f, 0.9f, 1.0f, 0.4f);

Engine::SetSlotMaterial(vehicle, bodySlot,  bodyMat);
Engine::SetSlotMaterial(vehicle, glassSlot, glassMat);
```

### Example 3 — Asset Sharing (Instancing)

```cpp
// Build one mesh with geometry
LPENTITY source = nullptr;
Engine::CreateMesh(&source);
LPSURFACE s = nullptr;
Engine::CreateSurface(&s, source);
// ... add vertices ...
Engine::FillBuffer(s);
Engine::SetSlotMaterial(source, 0, sharedMat);

// Create many instances sharing the same geometry
const int COUNT = 50;
LPENTITY instances[COUNT];
for (int i = 0; i < COUNT; ++i)
{
    Engine::CreateMesh(&instances[i]);
    Engine::ShareMeshAsset(source, instances[i]);
    Engine::PositionEntity(instances[i],
        (i % 10) * 3.0f, 0.0f, (i / 10) * 3.0f);
    Engine::SetSlotMaterial(instances[i], 0, sharedMat);
}
```

### Example 4 — Render-to-Texture Security Camera

```cpp
LPENTITY secCam = nullptr;
Engine::CreateCamera(&secCam);
Engine::PositionEntity(secCam, 10.0f, 5.0f, 0.0f);
Engine::LookAt(secCam, 0.0f, 0.0f, 0.0f);

LPRENDERTARGET feedRTT = nullptr;
Engine::CreateRenderTexture(&feedRTT, 256, 256);
Engine::SetRTTClearColor(feedRTT, 0.0f, 0.1f, 0.0f);

// Monitor mesh showing the feed
LPMATERIAL monitorMat = nullptr;
Engine::CreateMaterial(&monitorMat);

// In render loop:
Engine::SetRenderTarget(feedRTT, secCam);
Engine::RenderWorld();
Engine::ResetRenderTarget();

Engine::MaterialSetAlbedo(monitorMat, Engine::GetRTTTexture(feedRTT));

Engine::Cls(0, 0, 0);
Engine::RenderWorld();
Engine::Flip();
```

### Example 5 — Layer-based Multi-Camera Rendering

```cpp
LPENTITY worldCam = nullptr;
LPENTITY uiCam    = nullptr;
Engine::CreateCamera(&worldCam);
Engine::CreateCamera(&uiCam);

// Main camera sees everything except UI
Engine::CameraCullMask(worldCam, LAYER_DEFAULT | LAYER_FX | LAYER_SHADOW);

// UI camera sees only UI
Engine::SetCameraOrtho(uiCam, (float)Engine::GetWidth(), (float)Engine::GetHeight());
Engine::CameraCullMask(uiCam, LAYER_UI);

// Tag objects
Engine::EntityLayer(sceneMesh,   LAYER_DEFAULT);
Engine::EntityLayer(particleFX,  LAYER_FX);
Engine::EntityLayer(healthBarMesh, LAYER_UI);

// In render loop: render main scene, then UI pass
Engine::SetCamera(worldCam);
Engine::RenderWorld();

Engine::SetCamera(uiCam);
Engine::RenderWorld();
```
