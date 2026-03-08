# OYNAME-3DEngine — Technical Architecture Documentation

**Version:** February 2026  
**Renderer:** DirectX 11, Feature Level 11_0, Shader Model 5.0  
**Architecture:** Forward Renderer, Manager-based, BlitzBasic-inspired API

---

## Table of Contents

1. [Overview](#1-overview)
2. [Layer Model](#2-layer-model)
3. [Startup Sequence](#3-startup-sequence)
4. [Core Namespace](#4-core-namespace)
5. [GDXEngine](#5-gdxengine)
6. [Manager Architecture](#6-manager-architecture)
7. [Entity System](#7-entity-system)
8. [Mesh, MeshAsset and MeshRenderer](#8-mesh-meshasset-and-meshrenderer)
9. [Material and Shader System](#9-material-and-shader-system)
10. [Surface and Geometry](#10-surface-and-geometry)
11. [Texture and TexturePool](#11-texture-and-texturepool)
12. [Rendering Pipeline](#12-rendering-pipeline)
13. [Shadow Mapping](#13-shadow-mapping)
14. [Render-to-Texture](#14-render-to-texture)
15. [Skeletal Animation](#15-skeletal-animation)
16. [Scene Graph and Hierarchy](#16-scene-graph-and-hierarchy)
17. [Render Layers and Camera Culling](#17-render-layers-and-camera-culling)
18. [Timer](#18-timer)
19. [DirectX Buffer Rules](#19-directx-buffer-rules)
20. [Coding Rules Summary](#20-coding-rules-summary)
21. [Known Constraints and Pitfalls](#21-known-constraints-and-pitfalls)

---

## 1. Overview

OYNAME-3DEngine is a custom 3D engine written in C++ with a DirectX 11 backend. Its primary design goal is a clean, intuitive public API modeled after BlitzBasic, while keeping all DirectX complexity strictly encapsulated in internal systems. Game code never touches COM interfaces directly.

The engine uses a manager-based architecture. A central `GDXEngine` instance owns all managers and coordinates their initialization and shutdown. The public API surface is a single header, `gidx.h`, which contains only thin inline wrappers forwarding calls into the engine internals.

---

## 2. Layer Model

```
┌──────────────────────────────────────────────────────────────┐
│  Game Code                                                   │
│  (includes only gidx.h)                                      │
├──────────────────────────────────────────────────────────────┤
│  Engine:: namespace  (gidx.h)                                │
│  Thin inline wrappers – zero game-visible DirectX            │
├──────────────────────────────────────────────────────────────┤
│  Core:: namespace  (core.h / core.cpp)                       │
│  Window, COM, Timer, Paths, Shutdown                         │
├──────────────────────────────────────────────────────────────┤
│  GDXEngine  (central coordinator)                            │
│  ├── ObjectManager      creates / destroys all objects       │
│  ├── RenderManager      scene rendering, shadow, RTT         │
│  ├── ShaderManager      shader compilation and lookup        │
│  ├── BufferManager      GPU buffer allocation and updates    │
│  ├── InputLayoutManager vertex format → D3D input layout     │
│  └── TexturePool        SRV deduplication and indexing       │
├──────────────────────────────────────────────────────────────┤
│  GDXDevice    ID3D11Device / DeviceContext / SwapChain       │
│  GDXInterface DXGI adapter / output enumeration              │
└──────────────────────────────────────────────────────────────┘
```

Two namespaces are used:

- `Engine::` — all public game-facing API functions (e.g. `Engine::CreateMesh()`, `Engine::RenderWorld()`)
- `Core::` — bootstrap and lifecycle only (window creation, timing, shutdown)

Internal engine classes carry the `GDX` prefix (`GDXEngine`, `GDXDevice`, `GDXInterface`) and use no namespace. All other domain objects use plain PascalCase (`Mesh`, `Camera`, `Material`, etc.).

---

## 3. Startup Sequence

The application entry point is `WinMain` in `main.cpp`. Startup happens in six steps:

**Step 1 — Core::Init**  
Creates the Win32 window, initializes COM, starts the `Timer` singleton, resolves the executable directory, and registers the window class. Returns `HWND`.

**Step 2 — Core::CreateEngine**  
Instantiates `GDXEngine`, which internally creates `GDXDevice`, enumerates DXGI adapters via `GDXInterface`, compiles the built-in standard shaders, and initializes all manager instances.

**Step 3 — Game Thread**  
The game's `main()` function runs on a dedicated thread, keeping the message pump responsive on the main thread.

**Step 4 — Message Loop**  
The main thread runs the standard Win32 message loop (`GetMessage` / `DispatchMessage`).

**Step 5 — WM_CLOSE / WM_DESTROY**  
`Windows::MainLoop(false)` signals the game thread to exit its loop. After `mainThread.join()`, all resources are released.

**Step 6 — Core::Shutdown**  
Releases `GDXEngine`, then `Timer`, then COM.

---

## 4. Core Namespace

`Core` is the bootstrap and lifecycle layer. It explicitly never includes `ID3D11Device` or `IDXGISwapChain`. Its responsibilities are:

- Creating and managing the application window
- COM initialization and teardown
- Providing the `Core::Desc` configuration struct that game code fills at startup
- Frame timing via `Core::BeginFrame()` and `Core::EndFrame()`
- Path resolution (`Core::GetExeDir()`, `Core::ResolvePath()`)
- Orderly shutdown of the entire engine stack

```cpp
Core::Desc desc;
desc.vsync       = true;
desc.debug       = true;
desc.windowed    = true;
desc.windowTitle = L"My Game";

HWND hwnd = Core::Init(hInst, WindowProc, desc);
Core::CreateEngine();
```

---

## 5. GDXEngine

`GDXEngine` is the central coordinator and the owner of all manager instances. It is a singleton accessible through the `Engine::engine` pointer (defined in `gdxengine.h`), but game code never accesses it directly — the `Engine::` wrappers in `gidx.h` do that internally.

GDXEngine owns by value:

```
ObjectManager       m_objectManager
RenderManager       m_renderManager
ShaderManager       m_shaderManager
InputLayoutManager  m_inputLayoutManager
BufferManager       m_bufferManager
TexturePool         m_texturePool
```

And by reference (public):

```
GDXDevice           m_device
GDXInterface        m_interface
```

On construction, `GDXEngine` compiles the two built-in standard shader pairs:

- `VertexShader.hlsl` + `PixelShader.hlsl` → registered as `ShaderKey::Standard`
- `VertexShaderSkinning.hlsl` + `SkinPixelShader.hlsl` → registered as `ShaderKey::StandardSkinned`

Shader file paths are defined as constants in `gdxengine.h`:

```cpp
#define VERTEX_SHADER_FILE          L"..\\..\\shaders\\VertexShader.hlsl"
#define PIXEL_SHADER_FILE           L"..\\..\\shaders\\PixelShader.hlsl"
#define VERTEX_SKINNING_SHADER_FILE L"..\\..\\shaders\\VertexShaderSkinning.hlsl"
```

Never hardcode shader paths inline.

---

## 6. Manager Architecture

### Dependency Hierarchy

```
GDXEngine  (central coordination)
├── ObjectManager      creates / destroys all objects
├── RenderManager      reads shader list via GetShaders()
├── ShaderManager      independent
├── BufferManager      independent
├── InputLayoutManager independent
└── TexturePool        independent
```

**Rules:**
- Managers do not communicate directly with each other.
- If a manager needs data from another manager, it receives it through a public getter — never via `friend` or direct member access.
- If a manager needs a hardware resource (`GDXDevice`), it receives it through the `GDXEngine` constructor — not through another manager.

### ObjectManager

Single owner of all engine objects: `Mesh`, `Camera`, `Light`, `Material`, `Shader`, `Surface`, `MeshAsset`. Every `Create*` function allocates and registers the object. Every `Delete*` function destroys the object and removes it from all internal containers. Game code never calls `new` or `delete` on engine objects directly.

`ObjectManager` maintains monotone `uint32_t` ID counters for `Shader` and `Material` objects. These IDs are used by the `RenderQueue` for state-sorted rendering.

```cpp
uint32_t m_nextShaderId   = 0;
uint32_t m_nextMaterialId = 0;
```

### RenderManager

Drives the two-pass render loop (shadow pass, normal pass). Owns the `RenderQueue` for opaque objects, the transparent frame list, the `ShadowMapTarget`, the `BackbufferTarget`, and the SRV binding cache. Receives the `ObjectManager` reference and `GDXDevice` reference in its constructor.

### ShaderManager

Compiles HLSL shader pairs, creates `Shader` objects, and maintains a map from `ShaderKey` to `LPSHADER`. The `GetShader()` function with no arguments returns the currently active default shader.

### BufferManager

All GPU buffer creation flows through `BufferManager`. It enforces the directx buffer rules: `DYNAMIC` + `Map/Unmap` for constant buffers, `DEFAULT` or `IMMUTABLE` + `UpdateSubresource` for static geometry.

### InputLayoutManager

Creates `ID3D11InputLayout` objects from vertex format flags (`D3DVERTEX_FLAGS`). Keyed on the flag combination so layouts are not duplicated.

### TexturePool

Centralizes `ID3D11ShaderResourceView` management. Every SRV is registered once and receives a stable `uint32_t` index. Duplicate SRVs (same pointer) are deduplicated. Provides three built-in fallback indices accessible at any time:

- `WhiteIndex()` — solid white (default albedo)
- `FlatNormalIndex()` — flat normal map (0.5, 0.5, 1.0)
- `OrmIndex()` — default ORM (occlusion=1, roughness=0.5, metallic=0)

---

## 7. Entity System

All scene objects — meshes, cameras, and lights — are `Entity` subclasses. The `Entity` base class provides:

- `Transform transform` — local position, rotation, scale
- `MatrixSet matrixSet` — view and projection matrices (for cameras and lights)
- `Viewport viewport` — for cameras and lights
- `EntityGpuData* gpuData` — pointer to the entity's constant buffer on the GPU

### Type Tag System

`Entity` carries a `EntityType` tag (`Mesh`, `Camera`, `Light`) that enables zero-overhead downcasting in hot paths without `dynamic_cast`:

```cpp
bool IsMesh()   const noexcept { return m_entityType == EntityType::Mesh; }
bool IsCamera() const noexcept { return m_entityType == EntityType::Camera; }
bool IsLight()  const noexcept { return m_entityType == EntityType::Light; }

Mesh*   AsMesh()   noexcept { return reinterpret_cast<Mesh*>(this); }
Camera* AsCamera() noexcept { return reinterpret_cast<Camera*>(this); }
Light*  AsLight()  noexcept { return reinterpret_cast<Light*>(this); }
```

Type tags are set in the constructor of each subclass and are never modified afterward.

### Entity Properties

Every entity carries the following state flags:

| Flag | Default | Meaning |
|---|---|---|
| `m_active` | `true` | Inactive entities skip Update, physics, and rendering |
| `m_visible` | `true` | Invisible entities skip rendering but continue updating |
| `m_castShadows` | `true` | Controls shadow pass participation |
| `m_layerMask` | `LAYER_DEFAULT` | Bitmask for camera culling |

### Frame-Update Flag

To avoid redundant constant buffer uploads for meshes with multiple surfaces, each mesh carries a `m_frameUpdated` flag. The first surface draw for a given frame uploads the world matrix buffer; subsequent surfaces of the same mesh skip the upload.

---

## 8. Mesh, MeshAsset and MeshRenderer

A `Mesh` is an entity that represents a renderable object in the scene. Its geometry is separated from its transform and material assignments into two distinct components.

### MeshAsset

`MeshAsset` is a pure geometry container. It holds a list of `Surface*` slots (non-owning pointers). The `ObjectManager` owns all `Surface` objects; `MeshAsset` only references them.

Multiple `Mesh` entities can share the same `MeshAsset` (e.g. for instanced props). Each entity then has its own transform and its own per-slot material assignments.

Tombstoning is used for slot removal (`RemoveSlot`): the slot entry is set to `nullptr` rather than erased, preserving all higher slot indices. This keeps slot indices stable across the lifetime of the asset.

```
MeshAsset
└── m_slots[]    (non-owning Surface* vector, may contain nullptr tombstones)
```

### MeshRenderer

`MeshRenderer` is the per-entity component that bridges the shared asset and the per-instance material state. It is a private member of `Mesh` and is not directly accessible from game code.

```
MeshRenderer
├── m_asset*          (non-owning pointer to the shared MeshAsset)
└── m_slotMaterials[] (per-slot Material* overrides for this entity)
```

### Three-Level Material Fallback

For every slot during rendering, the resolved material is determined by this priority chain:

1. `MeshRenderer::m_slotMaterials[slot]` — per-instance slot override (set via `SetSlotMaterial`)
2. `Surface::pMaterial` — material stored directly on the surface
3. Engine default material — the built-in standard material, always non-null

This chain guarantees that a draw call never submits a null material.

### Asset Co-Deletion

When `ObjectManager::DeleteMesh(mesh)` is called, it also deletes the `MeshAsset` if no other mesh references it. If a `MeshAsset` is shared between multiple meshes, it must be manually detached from the mesh being deleted before calling `DeleteMesh`, to avoid premature deletion. Use `Engine::DetachMeshAsset` before destruction in that case.

---

## 9. Material and Shader System

### Material

`Material` holds the CPU-side parameter state that maps to the `cbuffer MaterialBuffer` (register `b2`) in the pixel shader. It carries no DirectX COM interfaces itself — all GPU resources live in `MaterialGpuData` (a separate object managed by `BufferManager`).

Key material parameters:

| Parameter | Type | Description |
|---|---|---|
| `baseColor` | `XMFLOAT4` | Diffuse / albedo base color |
| `specularColor` | `XMFLOAT4` | Specular color (legacy) |
| `emissiveColor` | `XMFLOAT4` | Emissive color + intensity |
| `uvTilingOffset` | `XMFLOAT4` | UV tiling (xy) and offset (zw) |
| `metallic` | `float` | PBR metallic factor (0–1) |
| `roughness` | `float` | PBR roughness factor (0–1) |
| `normalScale` | `float` | Normal map intensity |
| `occlusionStrength` | `float` | Ambient occlusion strength |
| `alphaCutoff` | `float` | Threshold for alpha test |
| `blendMode` | `int` | Secondary texture blend mode (0–5) |
| `flags` | `uint32_t` | Feature flags (PBR, normal map, ORM, etc.) |

**Material flags** (selected):

| Flag | Meaning |
|---|---|
| `MF_USE_PBR` | Enables PBR shading path |
| `MF_USE_NORMAL_MAP` | Activates normal mapping |
| `MF_USE_ORM_MAP` | Activates ORM texture lookup |
| `MF_ALPHA_TEST` | Enables alpha cutoff discarding |
| `MF_TRANSPARENT` | Routes material to the transparent pass |
| `MF_RECEIVE_SHADOWS` | Enables shadow map lookup in the pixel shader |

`Material` carries its own `uint32_t id`, assigned by `ObjectManager` from a monotone counter. This ID is used by `RenderQueue::Sort()` for state-sorted batching.

### Shader

`Shader` compiles a vertex+pixel shader pair and creates the matching `ID3D11InputLayout` from the `D3DVERTEX_FLAGS` combination. After the input layout is created, the shader blobs are released. `Shader` carries its own `uint32_t id` for sort-key batching.

Vertex format flags (`D3DVERTEX_FLAGS`):

| Flag | Meaning |
|---|---|
| `D3DVERTEX_POSITION` | XYZ position |
| `D3DVERTEX_NORMAL` | Surface normal |
| `D3DVERTEX_COLOR` | Vertex color (RGBA) |
| `D3DVERTEX_TEX1` | First UV channel |
| `D3DVERTEX_TEX2` | Second UV channel |
| `D3DVERTEX_TANGENT` | Tangent vector (float4 with handedness) |
| `D3DVERTEX_BONE_INDICES` | Four bone indices per vertex (uint4) |
| `D3DVERTEX_BONE_WEIGHTS` | Four bone weights per vertex (float4) |

### Constant Buffer Register Map

These assignments are fixed across all shaders. Custom buffers must use `b3` or higher.

| Register | Buffer | Updated by |
|---|---|---|
| `b0` | `MatrixBuffer` | Per entity — world / view / projection matrices |
| `b1` | `LightBuffer` | Per frame — all active lights (up to 32) |
| `b2` | `MaterialBuffer` | Per material — PBR parameters and flags |
| `b4` | `BoneBuffer` | Per skinned mesh — up to 128 bone matrices |

---

## 10. Surface and Geometry

A `Surface` is the atomic unit of renderable geometry. It owns CPU-side vertex arrays (positions, normals, tangents, colors, UV1, UV2, bone indices, bone weights) and an index array. All GPU resources are stored in a `SurfaceGpuBuffer` (accessible via `surface->gpu`).

After filling vertex and index data via the `Engine::AddVertex` / `Engine::AddTriangle` API, the game code calls `Engine::FillBuffer(surface)` to upload all data to the GPU. From that point on, the CPU-side arrays remain valid and can be used to update dynamic geometry via `Engine::UpdateVertexBuffer` / `Engine::UpdateNormalBuffer` / `Engine::UpdateColorBuffer`.

`SurfaceGpuBuffer` stores individual `ID3D11Buffer*` objects for each vertex stream (position, normal, tangent, color, UV1, UV2, bone indices, bone weights, index buffer). Multi-stream vertex binding is performed by `Dx11RenderBackend` at draw time.

Tangent vectors are computed on the CPU by `Surface::ComputeTangents()`. This function is called automatically by `FillBuffer` when the shader requires tangents (`D3DVERTEX_TANGENT` flag) and the tangent count does not match the vertex count. Shader-side TBN re-orthogonalization is not used — CPU-side computation already orthogonalizes the basis.

### Wireframe Mode

Individual surfaces can be rendered in wireframe mode without changing the rasterizer state globally:

```cpp
surface->gpu->SetWireframe(true);
```

---

## 11. Texture and TexturePool

Textures are loaded from disk via `Engine::LoadTexture` and returned as `LPTEXTURE` pointers. The texture object (`Texture` class) stores the `ID3D11Texture2D*`, `ID3D11ShaderResourceView*`, and `ID3D11SamplerState*`.

When a texture is assigned to a material via `Engine::MaterialSetAlbedo`, `MaterialSetNormal`, `MaterialSetORM`, or the legacy `MaterialTexture` slot API, the SRV is registered in the `TexturePool`. The pool returns a stable `uint32_t` index that the material stores. The pixel shader receives the active SRV array (bound as a flat array of individual texture slots) and uses these indices to look up the correct textures.

Dynamic textures (procedurally generated pixel data) are created via `Engine::CreateTexture`, modified pixel-by-pixel with `Engine::LockBuffer` / `Engine::SetPixel` / `Engine::UnlockBuffer`, and assigned to materials like any other texture.

---

## 12. Rendering Pipeline

### Frame Structure

Each frame follows this sequence:

```
Core::BeginFrame()
Engine::Cls(r, g, b)
Engine::UpdateWorld()
Engine::RenderWorld()
Engine::Flip()
Core::EndFrame()
```

### UpdateWorld

Iterates all active entities and calls `Entity::Update()`. This rebuilds each entity's world matrix from its local transform and parent chain, and uploads the updated matrix to the entity's constant buffer (`b0`) on the GPU.

### RenderWorld

`GDXEngine::RenderWorld()` delegates to `RenderManager::RenderScene()`, which runs the full two-pass render:

**Pass 1 — Shadow Pass** (`RenderShadowPass`)  
Renders all shadow-casting meshes into the `ShadowMapTarget` from the directional light's perspective. Uses a depth-only vertex shader (`ShaderBindMode::VS_ONLY`). The shadow map SRV is explicitly unbound before switching render targets to avoid SRV hazards.

**Pass 2 — Normal Pass** (`RenderNormalPass`)  
Clears the backbuffer (or active RTT), builds the render queue, sorts it, and flushes it in two sub-passes:

1. **Opaque sub-pass** — draws all non-transparent surfaces in Shader ID → Material ID order (minimizes GPU state changes)
2. **Transparent sub-pass** — draws transparent surfaces back-to-front (depth sorted)

### RenderQueue

`RenderQueue` is a flat list of `RenderCommand` objects. Each command carries all data needed for a single draw call: `Shader*`, `Material*`, `Mesh*`, `Surface*`, world matrix, and a `IRenderBackend*` pointer for the actual draw dispatch.

`RenderQueue::Sort()` orders commands by `shader->id` first, then `material->id`. This minimizes GPU state changes without pointer-truncation on 64-bit platforms. Both IDs are stable `uint32_t` values assigned by `ObjectManager`.

### SRV Binding Cache

`RenderManager` maintains `m_boundSRVs[7]`, a cached array of the last-bound SRVs for pixel shader slots `t0`–`t6`. Before each draw call, the backend compares the material's required SRVs against the cache and skips `PSSetShaderResources` calls for slots that are already bound with the correct SRV.

### Transparent Pass

Transparent materials (marked with `MF_TRANSPARENT`) are collected into `m_transFrame` as `(float depth, RenderCommand)` pairs. Depth is the world-space Z distance from the camera. After all opaques are drawn, `m_transFrame` is sorted descending by depth and flushed with alpha blending enabled.

---

## 13. Shadow Mapping

The engine supports single directional-light PCF shadow mapping.

`ShadowMapTarget` manages the shadow map render target: a depth-only texture rendered from the light's perspective. The shadow map resolution is configurable. PCF filtering (percentage closer filtering) is applied in the pixel shader via a comparison sampler.

Shadow map coverage is controlled per-light:

- `LightShadowOrthoSize(light, size)` — sets the orthographic projection size (world units). Smaller = sharper shadows, fewer objects covered.
- `LightShadowPlanes(light, near, far)` — sets the light camera's near and far planes. Tighter range = better depth precision, less shadow acne.
- `LightShadowFov(light, fovRadians)` — for perspective (spotlight) shadow cameras.

Shadow reception is per-material (`MaterialReceiveShadows`). Shadow casting is per-entity (`EntityCastShadows`).

Only one directional light can cast shadows. It must be registered explicitly:

```cpp
Engine::SetDirectionalLight(light);
```

---

## 14. Render-to-Texture

`RenderTextureTarget` encapsulates a full render target: color texture, depth buffer, viewport, and clear color. It implements the `IRenderTarget` interface.

The RTT workflow:

```cpp
LPRENDERTARGET rtt = nullptr;
Engine::CreateRenderTexture(&rtt, 512, 512);
Engine::SetRTTClearColor(rtt, 0.0f, 0.0f, 0.0f);

Engine::SetRenderTarget(rtt, rttCamera);  // all RenderWorld() calls now render here
Engine::RenderWorld();
Engine::ResetRenderTarget();              // restore backbuffer

LPTEXTURE rttTex = Engine::GetRTTTexture(rtt);
Engine::MaterialSetAlbedo(screenMat, rttTex);
```

A separate `rttCamera` can be supplied to render the RTT pass from a different viewpoint than the main scene camera.

SRV hazards between the RTT and the shadow pass are avoided by explicit unbinding before render target switches.

---

## 15. Skeletal Animation

Skeletal animation uses a separate skinning shader pair (`VertexShaderSkinning.hlsl` / `SkinPixelShader.hlsl`) registered as `ShaderKey::StandardSkinned`.

Bone data is per-vertex: four bone indices (`uint4`) and four bone weights (`float4`) stored in dedicated vertex streams. Bone weights must sum to 1.0.

At runtime, an array of up to 128 `XMMATRIX` bone transforms is uploaded to the GPU via `Engine::SetEntityBoneMatrices`. On first call, the bone constant buffer (`b4`) is created on the mesh. Subsequent calls update it via `Map/Unmap`.

The skinning vertex shader blends vertex positions and normals using the standard linear blend skinning formula. The result is passed to the standard pixel shader (or the skinned pixel shader variant), which handles lighting and material evaluation identically to static meshes.

To create a skinned mesh:

1. Build geometry with `AddVertex` / `VertexNormal` / `VertexTexCoord` as usual.
2. Call `Engine::VertexBoneData(surface, vertexIndex, b0,b1,b2,b3, w0,w1,w2,w3)` for each vertex.
3. Call `Engine::FillBuffer(surface)` to upload all streams.
4. Create the material with `Engine::CreateSkinnedMaterial`.
5. Each frame, call `Engine::SetEntityBoneMatrices(entity, matrices, count)` with the current pose.

---

## 16. Scene Graph and Hierarchy

`Entity` supports an unlimited parent-child hierarchy. When a parent is assigned, the child's local transform remains stored as-is. `Entity::GetWorldMatrix()` recursively multiplies the local transform with the parent's world matrix.

```
world = local * parent->GetWorldMatrix()
```

Children do not own parents, and parents do not own children — the `ObjectManager` owns all entities. Setting a parent does not transfer ownership.

The `Space` enum controls whether move and rotate operations apply in local or world space:

```cpp
enum class Space { Local, World };
Engine::MoveEntity(wheel, 1.5f, 0.0f, 0.0f);                 // Local (default)
Engine::MoveEntity(wheel, 1.5f, 0.0f, 0.0f, Space::World);   // World
```

---

## 17. Render Layers and Camera Culling

Render layers are 32-bit bitmasks. Each entity carries a `layerMask`; each camera carries a `cullMask`. An entity is visible to a camera only if `(entity->layerMask & camera->cullMask) != 0`.

Predefined layer constants (from `RenderLayers.h`):

| Constant | Bit | Typical use |
|---|---|---|
| `LAYER_DEFAULT` | 0 | Standard scene objects |
| `LAYER_UI` | 1 | HUD / screen-space elements |
| `LAYER_REFLECTION` | 2 | Reflection pass objects |
| `LAYER_SHADOW` | 3 | Shadow-only objects |
| `LAYER_FX` | 4 | Particles and effects |
| `LAYER_ALL` | all bits | No culling (default camera mask) |

---

## 18. Timer

`Timer` is a singleton (`Timer::GetInstance()`). It supports two time modes:

- `TimeMode::VARIABLE_TIMESTEP` — raw delta time each frame (default)
- `TimeMode::FIXED_TIMESTEP` — fixed 60 Hz steps with spiral-of-death protection (max 5 steps per frame)

Static getters for game code:

| Function | Description |
|---|---|
| `Timer::GetDeltaTime()` | Seconds elapsed since the last frame |
| `Timer::GetFPS()` | Current frames per second |
| `Timer::GetFixedStep()` | Fixed step duration (1/60 s) |
| `Timer::GetFixedSteps()` | Number of fixed steps to process this frame |

Always use `Timer::GetDeltaTime()`, never `Time.DeltaTime()`.

---

## 19. DirectX Buffer Rules

| Usage Flag | Allowed Update Method | Forbidden |
|---|---|---|
| `D3D11_USAGE_DEFAULT` | `UpdateSubresource()` | `Map` / `Unmap` |
| `D3D11_USAGE_DYNAMIC` + `D3D11_CPU_ACCESS_WRITE` | `Map(WRITE_DISCARD)` + `Unmap` | `UpdateSubresource` |
| `D3D11_USAGE_IMMUTABLE` | Only at creation time | Everything after creation |

Practical rules:
- Constant buffers are always `DYNAMIC` + `Map/Unmap`
- Static vertex / index buffers are `DEFAULT` or `IMMUTABLE` + `UpdateSubresource`
- Dynamic vertex / index buffers (procedural geometry) are `DYNAMIC` + `Map/Unmap`

---

## 20. Coding Rules Summary

- Internal engine classes use the `GDX` prefix + PascalCase: `GDXEngine`, `GDXDevice`
- Domain objects use plain PascalCase: `Mesh`, `Camera`, `Material`, `Surface`
- Manager classes use PascalCase + `Manager` suffix: `ObjectManager`, `RenderManager`
- All methods use PascalCase: `CreateMesh()`, `DeleteCamera()`, `GetPosition()`
- Exception: STL-compatible methods stay lowercase: `begin()`, `end()`, `size()`
- Instance members: `m_` prefix + camelCase: `m_hwnd`, `m_screenWidth`
- Static members: `s_` prefix + camelCase: `s_instance`
- Constants / `#define`: ALL_CAPS: `VERTEX_SHADER_FILE`, `SCREEN_DEPTH`
- Scoped enums: `enum class` + PascalCase: `enum class RenderQueueType { Opaque, Transparent }`
- Legacy enums / flags: ALL_CAPS: `D3DLIGHTTYPE`, `COLLISION`
- `Core::` must never include `ID3D11Device` or `IDXGISwapChain`
- Never write `using namespace DirectX;` in a header file
- Shader file paths are defined as constants in `gdxengine.h`, never hardcoded inline
- Comments are written in English. Existing German comments remain but are not continued
- One function, one responsibility. If a comment contains the word "and", the function should be split
- No global variables except `Engine::engine`
- Per-frame `Debug::Log` output must be guarded by a debug flag

---

## 21. Known Constraints and Pitfalls

**Dynamic texture array indexing is unsupported.** `Texture2D gTex[16]` cannot be dynamically indexed in HLSL SM5.0 under Feature Level 11_0. All texture bindings use individual named slots (`t0`–`t6`).

**Constant buffer register conflicts are silent.** Assigning two buffers to the same register produces no compiler error but corrupts rendering. Verify register assignments across all shaders when adding new buffers. Custom buffers must use `b3` or higher.

**SRV hazards between passes.** The shadow map SRV and RTT SRVs must be explicitly unbound before switching render targets. Leaving an SRV bound while its underlying texture is also bound as a render target produces undefined behavior in DX11.

**MeshAsset co-deletion.** `DeleteMesh` deletes the associated `MeshAsset` if no other mesh references it. Shared assets must be detached before deletion. Call `DetachMeshAsset` before `DeleteMesh` when sharing.

**Shader-side TBN re-orthogonalization is redundant.** `Surface::ComputeTangents()` already orthogonalizes the TBN basis on the CPU. Applying Gram-Schmidt in the vertex or pixel shader introduces drift and should be avoided.

**Tombstoning in MeshAsset.** `RemoveSlot` sets the slot pointer to `nullptr` rather than erasing the entry. Slot indices are therefore permanent for the lifetime of the asset. Code iterating `MeshAsset::GetSlots()` must handle `nullptr` entries.

**Per-camera debug guards.** Debug output guards that track "logged once" state for multiple cameras must use `unordered_map<void*, size_t>` keyed on the camera pointer. A single static bool breaks when multiple cameras are active.

**F0 calculation for PBR metallic.** The Fresnel base reflectance must be computed from the final composited albedo (after all detail map blending is applied), not from a pre-blend approximation.

**`UpdateSubresource` is invalid for `DYNAMIC` buffers.** Only `Map/Unmap` (with `D3D11_MAP_WRITE_DISCARD`) is valid for `DYNAMIC` constant buffers and dynamic vertex buffers.

**Operator overloads for enums must be in headers.** Enum operator overloads defined in `.cpp` files are not visible across translation units and produce linker errors.
