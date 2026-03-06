# OYNAME-3DEngine

`gidx.h` - API Reference and Application Development

Complete function reference · Step-by-step guide · Code examples

Status: 2025 · Namespace `Engine` · `#include "gidx.h"`

## 1. Introduction

The file `gidx.h` is the only public interface of the OYNAME-3DEngine. It contains only inline functions in the `Engine` namespace and hides the entire DirectX 11 implementation behind simple, direct calls. The design follows the BlitzBasic model: an application calls functions and immediately gets the desired result, without managing COM interfaces, resource descriptions, or render states.

To write an application, a single include is enough. All types, all functions, and all constants are available afterward.

```cpp
#include "gidx.h"

// All engine functions are in the Engine namespace:
using namespace Engine; // optional for shorter code
```

## 2. Creating an Application - Step by Step

### 2.1 Structure of a `main()` Function

The engine clearly separates initialization and the game loop. `WinMain` in `main.cpp` automatically handles window management and engine creation. Your own `main()` function only contains `Engine::Graphics()`, scene creation, and the frame loop. A minimal skeleton looks like this:

```cpp
#include "gidx.h"

int main()
{
    // 1. Set graphics mode
    Engine::Graphics(1280, 720);            // Windowed mode
    // Engine::Graphics(1920, 1080, false); // Fullscreen

    // 2. Create camera
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 2.0f, -10.0f);

    // 3. Create light
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::TurnEntity(light, 45.0f, 0.0f, 0.0f);
    Engine::LightColor(light, 1.0f, 1.0f, 1.0f);
    Engine::SetDirectionalLight(light);
    Engine::SetAmbientColor(0.3f, 0.3f, 0.3f);

    // 4. Load texture
    LPTEXTURE tex = nullptr;
    Engine::LoadTexture(&tex, L"..\\media\\albedo.png");

    // 5. Create material
    LPMATERIAL mat = nullptr;
    Engine::CreateMaterial(&mat);
    Engine::MaterialSetAlbedo(mat, tex);

    // 6. Create mesh (custom geometry function)
    LPENTITY mesh = nullptr;
    Engine::CreateMesh(&mesh);
    LPSURFACE surf = nullptr;
    Engine::CreateSurface(&surf, mesh);
    // ... add vertices and triangles ...
    Engine::FillBuffer(surf);
    Engine::EntityMaterial(mesh, mat);
    Engine::PositionEntity(mesh, 0.0f, 0.0f, 5.0f);

    // 7. Game loop
    while (Windows::MainLoop())
    {
        Core::BeginFrame();
        float dt = (float)Timer::GetDeltaTime();

        Engine::TurnEntity(mesh, 45.0f * dt, 45.0f * dt, 0.0f);

        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }
    return 0;
}
```

### 2.2 The Frame Loop in Detail

Each frame follows a fixed order. `Core::BeginFrame()` must be called first so that timer values are valid. Then comes your own logic (movement, AI, input). `Engine::Cls()` clears the backbuffer. `Engine::UpdateWorld()` calculates all entity transformations and uploads constant buffers. `Engine::RenderWorld()` executes the shadow pass and normal pass. `Engine::Flip()` presents the backbuffer on the monitor. `Core::EndFrame()` completes the frame timing.

## 3. Graphics Initialization

### 3.1 Adapter and Output

If multiple graphics cards or monitors are present in the system, the desired adapter and output can be selected before calling `Graphics()`. `CountGfxDrivers()` returns the number of available adapters. `GfxDriverName()` returns the name of the current adapter. `SetGfxDriver(index)` selects an adapter. `SetOutput(index)` selects the output for fullscreen mode.

| **Function** | **Description** |
|---|---|
| `Graphics(w, h, windowed=true)` | Initializes the DirectX swap chain with the given resolution. `windowed=false` for fullscreen. |
| `CountGfxDrivers()` | Returns the number of graphics adapters in the system. |
| `GfxDriverName()` | Name of the currently selected adapter (e.g. NVIDIA GeForce RTX...). |
| `SetGfxDriver(index)` | Selects an adapter by index (0-based). Call before `Graphics()`. |
| `CountOutputs()` | Number of monitors connected to the current adapter. |
| `SetOutput(index)` | Selects the output monitor for fullscreen mode. |
| `CountGfxModes(output)` | Number of supported resolutions on the selected output. |
| `GfxModeWidth(mode)` | Width of a specific display mode in pixels. |
| `GfxModeHeight(mode)` | Height of a specific display mode in pixels. |
| `GetGfxModeFrequency(mode)` | Refresh rate of a display mode in Hz. |
| `GfxModeExists(w,h,freq)` | Checks whether a specific graphics mode is supported. |
| `GetWidth() / GetHeight()` | Current backbuffer resolution in pixels. |
| `SetVSync(interval)` | VSync: `1=enabled`, `0=disabled`. |
| `GetVSync()` | Returns the current VSync interval. |

## 4. Entity Management - Position, Rotation, Scaling

All functions in this section work with every entity type: meshes, cameras, and lights. `LPENTITY` is a pointer to `Entity`. All transformation functions are frame-rate independent if they are scaled with `Timer::GetDeltaTime()`.

### 4.1 Positioning

| **Function** | **Description** |
|---|---|
| `PositionEntity(e, x, y, z)` | Sets the position absolutely in local coordinates (or world coordinates if there is no parent). |
| `PositionEntity(e, XMVECTOR)` | Same as above, but with DirectX `XMVECTOR`. |
| `MoveEntity(e, x, y, z, space=Local)` | Moves relatively by the given vector. `Space::Local` or `Space::World`. |
| `EntityX/Y/Z(e)` | Returns the world position on the corresponding axis (`float`). |
| `EntityPosition(e)` | Returns the full world position as `XMVECTOR`. |
| `EntityLocalX/Y/Z(e)` | Local position without parent influence. |
| `EntityLocalPosition(e)` | Local position as `XMVECTOR`. |

### 4.2 Rotation and Orientation

| **Function** | **Description** |
|---|---|
| `RotateEntity(e, rx, ry, rz, space=Local)` | Sets the rotation absolutely in Euler angles (degrees). Replaces the previous rotation. |
| `RotateEntity(e, XMVECTOR quat)` | Sets the rotation by quaternion. |
| `TurnEntity(e, rx, ry, rz, space=Local)` | Rotates incrementally (adds to the existing rotation). Good for continuous rotation in the loop. |
| `LookAt(e, tx, ty, tz)` | Orients the entity toward a world point. The up vector is always `(0,1,0)`. |
| `LookAt(e, XMVECTOR target)` | Same as above with an `XMVECTOR` target position. |
| `ScaleEntity(e, x, y, z)` | Sets non-uniform scaling. All axes are independent. |

### 4.3 Hierarchy

| **Function** | **Description** |
|---|---|
| `SetEntityParent(child, parent)` | Attaches `child` as a child of `parent`. `parent=nullptr` corresponds to `DetachEntity`. |
| `DetachEntity(entity)` | Detaches `entity` from its parent. World matrix calculation then uses only local data. |
| `GetEntityParent(entity)` | Returns the parent pointer or `nullptr`. |

### 4.4 Visibility and Layers

| **Function** | **Description** |
|---|---|
| `EntityActive(e, bool)` | `false`: no update, no physics, no rendering. |
| `EntityActive(e)` | Returns the active state. |
| `ShowEntity(e, bool)` | `false`: update continues, only rendering is suppressed. |
| `EntityVisible(e)` | Returns the visibility state. |
| `EntityLayer(e, mask)` | Sets the layer bitmask. Combine `LAYER_DEFAULT`, `LAYER_UI`, etc. |
| `EntityLayer(e)` | Returns the current layer mask. |
| `EntityCastShadows(e, bool)` | `false`: mesh is skipped in the shadow pass. |

## 5. Camera

| **Function** | **Description** |
|---|---|
| `CreateCamera(&camera)` | Creates a camera with standard parameters: FOV 60°, aspect ratio from screen resolution, near 0.1, far 1000. |
| `SetCamera(camera)` | Sets the camera as the active render camera for the frame. |
| `CameraCullMask(cam, mask)` | Sets the cull mask: only entities with `(layer & mask) != 0` are rendered. |
| `CameraCullMask(cam)` | Returns the current cull mask. |

> Note: Camera position and rotation are controlled through the general entity functions `PositionEntity` and `TurnEntity`. `CreateCamera` automatically sets the camera as the active camera.

## 6. Light

| **Function** | **Description** |
|---|---|
| `CreateLight(&light, D3DLIGHT_DIRECTIONAL)` | Creates a directional light. Automatically creates constant buffers for light and matrix data. |
| `LightColor(light, r, g, b, a=1.0)` | Sets the diffuse color of the light. Values above `1.0` are possible for overbrightening. |
| `SetDirectionalLight(light)` | Registers this light as the shadow caster. Only one directional light can cast shadows. |
| `SetAmbientColor(r, g, b, a=1.0)` | Global ambient color of the entire scene. Affects all materials. |
| `MaterialReceiveShadows(mat, bool)` | `false`: material does not evaluate the shadow map (remains bright even in shadow). |
| `PositionLightAtCamera(light, cam, offset)` | Synchronizes light position and rotation with a camera (e.g. flashlight effect). |

## 7. Mesh and Geometry

### 7.1 Creating a Mesh

| **Function** | **Description** |
|---|---|
| `CreateMesh(&mesh)` | Creates an empty mesh entity and its GPU constant buffer. |
| `CreateSurface(&surf, mesh)` | Creates a new surface (sub-geometry) and adds it to the mesh. |
| `GetSurface(mesh)` | Returns the first surface. |
| `GetSurface(mesh, index)` | Returns the surface at index `i`. The index is stable (tombstoning). |
| `GetSurfaceCount(mesh)` | Number of slots in the `MeshAsset` (including tombstones). |
| `EntitySurface(mesh, index=0)` | Same as `GetSurface(mesh, index)`, alternative spelling. |

### 7.2 Adding Vertex Data

All `AddVertex`, `VertexNormal`, `VertexColor`, and `VertexTexCoord` calls must happen before `FillBuffer()`. After `FillBuffer()`, the CPU-side arrays still exist, but they can only be transferred to the GPU through `UpdateVertexBuffer()` or `UpdateColorBuffer()`.

| **Function** | **Description** |
|---|---|
| `AddVertex(surf, x, y, z)` | Adds a vertex with position (append). |
| `AddVertex(index, surf, XMFLOAT3)` | Inserts a vertex at a specific index. |
| `VertexNormal(surf, x, y, z)` | Sets the normal for the last added vertex (append). |
| `VertexNormal(surf, index, x, y, z)` | Sets the normal for a specific vertex index. |
| `VertexColor(surf, r, g, b)` | Sets the vertex color (`0-255`). Last vertex (append). |
| `VertexColor(surf, index, r, g, b)` | Sets the vertex color for a specific index. |
| `VertexTexCoord(surf, u, v)` | UV coordinate for UV channel 1 (albedo/normal/ORM). Append. |
| `VertexTexCoord2(surf, u, v)` | UV coordinate for channel 2 (lightmap/detail). Requires `D3DVERTEX_TEX2`. |
| `AddTriangle(surf, a, b, c)` | Adds a triangle through three vertex indices. |
| `FillBuffer(surf)` | Transfers all CPU data into GPU vertex and index buffers. Call once. |

### 7.3 Dynamic Buffer Updates

| **Function** | **Description** |
|---|---|
| `UpdateVertexBuffer(surf)` | Transfers changed vertex positions to the GPU. Only valid after `FillBuffer()`. |
| `UpdateColorBuffer(surf)` | Transfers changed vertex colors to the GPU. Only valid after `FillBuffer()`. |
| `SurfaceWireframe(surf, bool)` | Enables or disables wireframe mode for this surface. |
| `SurfaceWireframe(surf)` | Returns the current wireframe state. |

### 7.4 Asset Sharing

`ShareMeshAsset(source, target)` lets two meshes share the same geometry. Each instance has its own transform and its own materials. The geometry exists only once in GPU memory.

```cpp
LPENTITY original = nullptr, copy = nullptr;
Engine::CreateMesh(&original);
// ... build geometry for original ...

Engine::CreateMesh(&copy);
Engine::ShareMeshAsset(original, copy);   // copy uses geometry from original

// Now position them independently:
Engine::PositionEntity(original, -3.0f, 0.0f, 5.0f);
Engine::PositionEntity(copy,      3.0f, 0.0f, 5.0f);

// Before DeleteMesh: secure the asset pointer if the other instance stays alive:
// copy->AsMesh()->meshRenderer.asset = nullptr;
// Engine::DeleteMesh(&copy);
```

## 8. Material

### 8.1 Creating and Assigning Materials

| **Function** | **Description** |
|---|---|
| `CreateMaterial(&mat, shader=nullptr)` | Creates a material with the standard shader (when `nullptr` is passed) and initializes the GPU constant buffer. |
| `EntityMaterial(mesh, mat)` | Assigns the material to the entire mesh (all slots). |
| `SurfaceMaterial(surf, mat)` | Assigns the material to a single surface (per-surface override). |
| `SetSlotMaterial(mesh, slot, mat)` | Sets a material for a slot index directly in `MeshRenderer`. For asset sharing with different materials per instance. |

### 8.2 Textures

| **Function** | **Description** |
|---|---|
| `LoadTexture(&tex, L"path")` | Loads a texture from a file (BMP, PNG, DDS). Checks file existence before loading. |
| `MaterialSetAlbedo(mat, tex)` | Assigns the albedo texture. Registers the SRV in the `TexturePool` (index `0 = Albedo`). |
| `MaterialSetNormal(mat, tex)` | Assigns the normal map. Automatically sets `MF_USE_NORMAL_MAP`. |
| `MaterialSetORM(mat, tex)` | Assigns the ORM texture (`R=Occlusion, G=Roughness, B=Metallic`). Sets `MF_USE_ORM_MAP`. |
| `MaterialSetDecal(mat, tex)` | Assigns an optional decal texture (slot 3). |
| `MaterialSetOcclusion(mat, tex)` | Separate occlusion texture (independent of ORM). |
| `MaterialSetRoughness(mat, tex)` | Separate roughness texture. |
| `MaterialSetMetallic(mat, tex)` | Separate metallic texture. |
| `MaterialTexture(mat, tex, slot=0)` | Legacy API: assigns a texture through a slot number. `0=Albedo`, `1=Normal`, `2=ORM`, `3=Decal`. |

### 8.3 Color and Classic Properties

| **Function** | **Description** |
|---|---|
| `MaterialColor(mat, r, g, b, a=1.0)` | Base color / tint factor of the material. |
| `MaterialSpecularColor(mat, r, g, b)` | Specular color in legacy shading mode. |
| `MaterialShininess(mat, value)` | Shininess / specular exponent in legacy mode. |
| `MaterialDoubleSided(mat, bool)` | `true`: disables backface culling for this material. |

### 8.4 PBR Parameters

PBR (physically based rendering) must be explicitly enabled with `MaterialUsePBR(mat, true)`. The standard shader then uses Cook-Torrance GGX instead of Blinn-Phong. PBR and legacy mode can be used simultaneously on different materials within the same frame.

| **Function** | **Description** |
|---|---|
| `MaterialUsePBR(mat, bool)` | `true`: enables PBR lighting (Cook-Torrance GGX). `false`: Blinn-Phong (standard). |
| `MaterialMetallic(mat, value)` | Metallic factor `0.0..1.0`. `0=dielectric`, `1=metallic`. |
| `MaterialRoughness(mat, value)` | Roughness `0.0..1.0`. `0=mirror`, `1=fully diffuse`. |
| `MaterialNormalScale(mat, scale)` | Scaling of the normal map influence. `1.0=full`, `0.0=flat`. |
| `MaterialOcclusionStrength(mat, strength)` | Strength of ambient occlusion `0.0..1.0`. |
| `MaterialEmissiveColor(mat, r,g,b, intensity=1.0)` | Self-emissive emissive color. `intensity` multiplies `rgb`. Added to the final pixel result. |

### 8.5 Transparency and Alpha

| **Function** | **Description** |
|---|---|
| `MaterialTransparent(mat, bool)` | `true`: material is placed in the transparent queue and rendered sorted back-to-front (true alpha blending). |
| `MaterialAlphaTest(mat, bool)` | `true`: alpha test mode. Pixels below `alphaCutoff` are discarded (no blending, no depth-sorting issue). |
| `MaterialAlphaCutoff(mat, cutoff)` | Alpha threshold `0..1` for `MaterialAlphaTest`. Automatically sets alpha test to `true`. |

## 9. Shader

The standard shader (`VertexShader.hlsl + PixelShader.hlsl`) is loaded automatically by the engine. Custom shaders can be created when special vertex formats or shading effects are required.

| **Function** | **Description** |
|---|---|
| `CreateShader(&sh, vsFile, vsEntry, psFile, psEntry, flags)` | Compiles vertex and pixel shaders from HLSL files. `flags` is a `DWORD` combination of the vertex format flags. |
| `CreateVertexFlags(pos, nrm, clr, uv1, uv2, tng)` | Creates a `DWORD` mask from boolean parameters for position, normal, color, UV1, UV2, tangent. |
| `CreateMaterial(&mat, shader)` | Creates a material explicitly bound to this shader (instead of the standard shader). |

```cpp
LPSHADER myShader = nullptr;

DWORD flags = Engine::CreateVertexFlags(
    true,   // Position (always true)
    true,   // Normal
    false,  // Color
    true,   // UV1
    true,   // UV2
    true    // Tangent (for normal mapping)
);

Engine::CreateShader(&myShader,
    L"shaders/VertexShader.hlsl", "VS",
    L"shaders/PixelShader.hlsl",  "PS",
    flags);

LPMATERIAL mat = nullptr;
Engine::CreateMaterial(&mat, myShader);
```

## 10. Textures - Procedural and Pixel Access

| **Function** | **Description** |
|---|---|
| `CreateTexture(&tex, width, height)` | Creates an empty writable texture in CPU access mode. |
| `LockBuffer(tex)` | Locks the texture buffer for CPU write access. Must be called before `SetPixel`. |
| `SetPixel(tex, x, y, r, g, b, alpha)` | Writes one pixel (`RGBA`, each `0-255`) at position `x,y`. |
| `GetColor(tex, x, y)` | Reads the pixel color back as a `Color(r,g,b,alpha)` struct. |
| `UnlockBuffer(tex)` | Releases the lock and transfers the changes to the GPU. |

## 11. Render-to-Texture (RTT)

RTT makes it possible to render an entire scene into a texture, which can then be used as the albedo of another object. Typical use cases are mirrors, security cameras, minimap rendering, or post-processing effects.

| **Function** | **Description** |
|---|---|
| `CreateRenderTexture(&rtt, width, height)` | Creates an RTT target with its own D3D11 render target view and depth buffer. |
| `SetRenderTarget(rtt, camera=nullptr)` | Redirects all subsequent `RenderWorld()` calls into the RTT. `camera=nullptr` uses the active main camera. |
| `ResetRenderTarget()` | Restores the backbuffer as the active render target. |
| `GetRTTTexture(rtt)` | Returns an `LPTEXTURE` wrapper for the RTT texture. Can be used directly with `MaterialSetAlbedo`. |
| `SetRTTClearColor(rtt, r,g,b,a=1.0)` | Sets the clear color of the RTT target (default: black). |
| `ReleaseRenderTexture(&rtt)` | Releases all D3D11 resources of the RTT. |

```cpp
// Mirror example:
LPRENDERTARGET mirrorRTT = nullptr;
Engine::CreateRenderTexture(&mirrorRTT, 512, 512);
Engine::SetRTTClearColor(mirrorRTT, 0.1f, 0.1f, 0.15f);

// Material for the mirror surface:
LPMATERIAL mirrorMat = nullptr;
Engine::CreateMaterial(&mirrorMat);
Engine::MaterialSetAlbedo(mirrorMat, Engine::GetRTTTexture(mirrorRTT));

// In the frame loop:
Engine::SetRenderTarget(mirrorRTT);    // Render into RTT
Engine::RenderWorld();
Engine::ResetRenderTarget();           // Back to the backbuffer
Engine::RenderWorld();                 // Normal frame
```

## 12. Skeletal Animation

| **Function** | **Description** |
|---|---|
| `VertexBoneData(surf, v, b0,b1,b2,b3, w0,w1,w2,w3)` | Sets bone indices and weights for vertex `v`. Call before `FillBuffer()`. The weights should add up to `1.0`. |
| `SetEntityBoneMatrices(mesh, matrices, count)` | Loads an array of `XMMATRIX` bone transformations onto the GPU (max. 128 bones, register `b4`). Call every frame. |

## 13. Collision

| **Function** | **Description** |
|---|---|
| `EntityCollisionMode(mesh, mode)` | Sets the collision mode (`COLLISION` enum). Activates OBB calculation. |
| `EntityCollision(mesh1, mesh2)` | Checks OBB collision between two meshes. Returns `true` if there is overlap. |
| `EntityOBB(mesh)` | Returns a pointer to `BoundingOrientedBox`. For direct DirectX collision calculations. |

## 14. Debug Output

| **Function** | **Description** |
|---|---|
| `DebugPrintMesh(entity)` | Outputs the internal structure of a mesh entity as a tree: transform, `MeshAsset`, slots, resolved materials. |
| `DebugPrintScene()` | Calls `DebugPrintMesh` for all meshes in the scene. Useful after scene setup for verification. |

> Note: The engine coding rule requires all `Debug::Log()` calls to begin with the file name. Example: `Debug::Log("game.cpp: Mesh created");`

## 15. Complete Example - PBR Scene with Shadow Mapping

This example shows a complete setup with two PBR materials, a directional light with shadow mapping, a parent/child hierarchy, and frame-rate-independent animation.

```cpp
#include "gidx.h"

void CreateCube(LPENTITY* mesh, LPMATERIAL material);

int main()
{
    Engine::Graphics(1280, 720);

    // --- Camera ---
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 3.0f, -12.0f);
    Engine::LookAt(camera, 0.0f, 0.0f, 0.0f);

    // --- Light ---
    LPENTITY dirLight = nullptr;
    Engine::CreateLight(&dirLight, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(dirLight, 10.0f, 10.0f, -5.0f);
    Engine::LookAt(dirLight, 0.0f, 0.0f, 0.0f);
    Engine::LightColor(dirLight, 1.5f, 1.4f, 1.2f);
    Engine::SetDirectionalLight(dirLight);
    Engine::SetAmbientColor(0.2f, 0.2f, 0.25f);

    // --- Textures ---
    LPTEXTURE albedo = nullptr, normal = nullptr, orm = nullptr;
    Engine::LoadTexture(&albedo, L"..\\media\\albedo.png");
    Engine::LoadTexture(&normal, L"..\\media\\normal.png");
    Engine::LoadTexture(&orm,    L"..\\media\\orm.png");

    // --- Material A (PBR) ---
    LPMATERIAL matA = nullptr;
    Engine::CreateMaterial(&matA);
    Engine::MaterialUsePBR(matA, true);
    Engine::MaterialSetAlbedo(matA, albedo);
    Engine::MaterialSetNormal(matA, normal);
    Engine::MaterialSetORM(matA, orm);
    Engine::MaterialMetallic(matA, 0.8f);
    Engine::MaterialRoughness(matA, 0.3f);
    Engine::MaterialNormalScale(matA, 1.0f);

    // --- Meshes ---
    LPENTITY parent = nullptr, child = nullptr;
    CreateCube(&parent, matA);
    Engine::PositionEntity(parent, -2.0f, 0.0f, 5.0f);

    CreateCube(&child, matA);
    Engine::SetEntityParent(child, parent);
    Engine::PositionEntity(child, 3.0f, 0.0f, 0.0f);  // Relative to the parent

    Engine::DebugPrintScene();

    // --- Game loop ---
    while (Windows::MainLoop())
    {
        Core::BeginFrame();
        float dt = (float)Timer::GetDeltaTime();

        Engine::TurnEntity(parent, 0.0f, 30.0f * dt, 0.0f, Space::World);

        Engine::Cls(20, 20, 40);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }
    return 0;
}
```

## 16. Types and Constants

### 16.1 Pointer Types

| **Type** | **Meaning** |
|---|---|
| `LPENTITY` | `Entity*` - general pointer for mesh, camera, light |
| `LPMATERIAL` | `Material*` - pointer to a material object |
| `LPSURFACE` | `Surface*` - pointer to a sub-geometry unit |
| `LPTEXTURE` | `Texture*` - pointer to a loaded or procedurally created texture |
| `LPSHADER` | `Shader*` - pointer to a compiled shader |
| `LPRENDERTARGET` | `IRenderTarget*` - pointer to an RTT target or another render target |
| `LPLPTEXTURE` | `Texture**` - double pointer, used for `LoadTexture` / `CreateTexture` |

### 16.2 Important Enums and Constants

| **Constant / Enum** | **Values and Meaning** |
|---|---|
| `D3DLIGHT_DIRECTIONAL` | Light type: directional light (sun). Only type with shadow mapping support. |
| `Space::Local / Space::World` | Coordinate system for `MoveEntity` and `TurnEntity`/`RotateEntity`. |
| `LAYER_DEFAULT` | Standard layer bit (bit 0). All entities are on this layer by default. |
| `LAYER_ALL` | All bits set: `0xFFFFFFFF`. The camera sees all entities. |
| `D3DVERTEX_POSITION` | Vertex flag: position (always required). |
| `D3DVERTEX_NORMAL` | Vertex flag: normals for lighting. |
| `D3DVERTEX_TEX1` | Vertex flag: first UV coordinate. |
| `D3DVERTEX_TEX2` | Vertex flag: second UV coordinate (detail/lightmap). |
| `D3DVERTEX_TANGENT` | Vertex flag: tangents for normal mapping. |
| `D3DVERTEX_BONE_INDICES/WEIGHTS` | Vertex flags for skeletal animation. |
