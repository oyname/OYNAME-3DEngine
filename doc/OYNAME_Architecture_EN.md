# OYNAME-3DEngine

Technical Architecture Documentation

DirectX 11 · Feature Level 11_0 · Shader Model 5.0 · Forward Renderer

Status: 2025 · Internal / Development

## 1. Overview and Design Philosophy

OYNAME-3DEngine is a 3D graphics engine developed in C++ that uses DirectX 11 with Feature Level 11_0 and Shader Model 5.0. The central design goal is a BlitzBasic-inspired user API that hides the entire complexity of DirectX behind clear, direct function calls. Developers do not write DirectX resource management, COM interfaces, or HLSL register configuration - all of that is handled by the engine.

The following core principles shape the architecture:

| **Principle** | **Implementation** |
|---|---|
| Intuitive API | All public functions in `gidx.h` are inline wrappers in the `Engine` namespace. No DirectX knowledge required. |
| Manager architecture | Every system domain (objects, rendering, shaders, buffers, textures) is managed by a dedicated manager class. |
| Separation of geometry and rendering | `MeshAsset` stores pure geometry data. `MeshRenderer` couples geometry with materials per slot. |
| Backward compatibility | New systems (PBR, RTT, TexturePool) extend the API as opt-in features. Existing code continues to work unchanged. |
| Entity type system | Type tags replace `dynamic_cast` in the hot path. `IsMesh`/`IsCamera`/`IsLight` with static cast helper methods. |

## 2. System Structure

### 2.1 Startup Sequence

The engine is initialized in `main.cpp` through two layers. `Core::Init()` sets up the Win32 window, COM, and the timer. `Core::CreateEngine()` instantiates the `GDXEngine` singleton and initializes all manager classes. The actual game loop runs on a separate thread, while the main thread handles the Win32 message loop.

```cpp
// WinMain - initialization order:
Core::Init(hInst, WindowProc, desc);   // Window + COM + Timer
Core::CreateEngine();                  // GDXEngine singleton
std::thread gameThread([]{ main(); }); // Game loop thread
// Win32 message loop on the main thread
```

### 2.2 GDXEngine (Central Singleton)

`GDXEngine` is the core of the engine. It is designed as a singleton and is reachable through `Engine::engine` as a global pointer. `GDXEngine` owns and coordinates all manager instances. No code outside the engine internals communicates directly with the managers - exclusively through the `gidx.h` API.

| **Manager Reference** | **Purpose** |
|---|---|
| `m_objectManager (GetOM())` | Central ownership and lifecycle management of all entities, materials, shaders, surfaces, and mesh assets. |
| `m_renderManager (GetRM())` | Construction and execution of the render queue, shadow pass, transparent pass, RTT support. |
| `m_shaderManager (GetSM())` | Shader compilation, input layout creation, shader registry. |
| `m_bufferManager (GetBM())` | Creation and update of all DirectX buffers (vertex, index, constant). |
| `m_texturePool (GetTP())` | Central SRV deduplication, stable numeric texture indices, default fallback textures. |
| `m_inputLayoutManager (GetILM())` | Creation of D3D11 input layouts based on vertex format flags. |
| `m_device (GDXDevice)` | Encapsulates `ID3D11Device`, `ID3D11DeviceContext`, and `IDXGISwapChain`. Access via `m_device.GetDevice()`. |
| `m_interface (GDXInterface)` | Adapter enumeration, display mode queries, output management. |

### 2.3 File Structure

| **Path** | **Content** |
|---|---|
| `include/gidx.h` | Complete public API (`Engine` namespace, all functions inline) |
| `include/gdxengine.h` | `GDXEngine` class, singleton declaration, all managers |
| `include/Entity.h` | Base `Entity` class with transform, type system, hierarchy |
| `include/Mesh.h / MeshAsset.h / MeshRenderer.h` | Mesh component architecture |
| `include/Material.h` | Material properties, flags, PBR data |
| `include/RenderManager.h` | Render queue, shadow pass, RTT control |
| `include/TexturePool.h` | SRV pool with deduplication and default textures |
| `shaders/` | `VertexShader.hlsl`, `PixelShader.hlsl`, skinning variants |
| `examples/` | Reference implementations for all engine features |
| `src/` | Implementation files of all engine systems |

## 3. Entity System

### 3.1 Base Class `Entity`

All scene-related objects - meshes, cameras, lights - inherit from `Entity`. `Entity` contains a transform (position, rotation, scaling), a viewport structure, `MatrixSet` for constant buffer data, and optional GPU resources via `EntityGpuData`.

The state of an entity is controlled through three orthogonal flags: `active` (controls update and rendering completely), `visible` (update continues, only rendering is suppressed), and `layerMask` (bitmask for camera culling). `EntityCastShadows` separately controls participation in the shadow pass.

### 3.2 Type System - No `dynamic_cast` in the Hot Path

Each entity carries an `EntityType` tag (`Mesh=1`, `Camera=2`, `Light=3`). The methods `IsMesh()`, `IsCamera()`, and `IsLight()` check this tag in `O(1)` without RTTI. `AsMesh()`, `AsCamera()`, and `AsLight()` perform `reinterpret_cast` after `IsMesh()` has returned true. This is safe because the types are set correctly in the construction chain and are never changed.

### 3.3 Hierarchy - Parent/Child

Entities can be nested hierarchically without limitation. `SetEntityParent(child, parent)` registers `child` as a child and `parent` as the parent object. The local transform remains unchanged; `GetWorldMatrix()` computes the full world matrix through recursive multiplication along the parent chain. `Space::Local` and `Space::World` control in which coordinate system `MoveEntity` and `RotateEntity` operate.

## 4. Mesh Architecture

### 4.1 Component Model

The mesh system is built in three layers. `MeshAsset` contains pure geometry as a vector of surface slots, without transform and without material. `Surface` contains the actual vertex and index data (position, normal, tangent, color, UV1, UV2, bone data) as well as a GPU buffer wrapper. `MeshRenderer` couples a `MeshAsset` with a material array per slot and belongs to the mesh entity.

| **Class** | **Role and Ownership** |
|---|---|
| `MeshAsset` | Geometry data container. Non-owning pointers to surfaces. Can be shared by multiple `MeshRenderer` instances (`ShareMeshAsset`). |
| `Surface` | Single sub-geometry with vertex/index arrays and GPU buffer (`SurfaceGpuBuffer`). Ownership lies with the `ObjectManager`. |
| `MeshRenderer` | Link between `MeshAsset` and materials. `slotMaterials[]` per slot. Material resolution: `slotMaterials[i]` -> engine default material. |
| `Mesh (Entity)` | Carrier of `MeshRenderer`, transform, OBB, skinning data, collision mode. |

### 4.2 Asset Sharing

`ShareMeshAsset(source, target)` lets two mesh entities reference the same `MeshAsset` instance. The geometry exists only once in GPU memory. Each instance has its own `MeshRenderer` and therefore its own material per slot, its own visibility, and its own transform. When deleting a mesh that carries a shared asset, `asset` must be set to `nullptr` before calling `DeleteMesh()` to prevent double deletion.

### 4.3 Tombstoning When Removing Slots

`RemoveSlot` on a `MeshAsset` replaces the slot with `nullptr` instead of compacting the vector. This keeps all existing slot indices stable as long as the asset is active. `NumActiveSlots()` counts only non-null slots. The renderer skips tombstone slots automatically.

## 5. Material System

### 5.1 `MaterialData` Layout

`MaterialData` is a C struct that exactly matches the `cbuffer MaterialBuffer` (register `b2`) in the pixel shader. It is 16-byte aligned. The struct contains classic fields (`baseColor`, `specularColor`), PBR fields (`metallic`, `roughness`, `normalScale`, `occlusionStrength`), emissive and UV tiling data, as well as a flags bitfield and a transparent `BlendMode` value.

### 5.2 Material Flags

| **Flag** | **Effect** |
|---|---|
| `MF_ALPHA_TEST (Bit 0)` | Alpha values below `alphaCutoff` are discarded. For foliage, fences, transparent surfaces without blending. |
| `MF_USE_NORMAL_MAP (Bit 3)` | The normal map is evaluated in the pixel shader. Set automatically by `MaterialSetNormal`. |
| `MF_USE_ORM_MAP (Bit 4)` | The ORM texture (occlusion/roughness/metallic combined) is read. Set by `MaterialSetORM`. |
| `MF_USE_EMISSIVE (Bit 5)` | Emissive color is added. Activated by `MaterialEmissiveColor`. |
| `MF_TRANSPARENT (Bit 6)` | Material goes into the transparent queue and is rendered sorted back-to-front. |
| `MF_SHADING_PBR (Bit 10)` | Switches to the PBR lighting model (Cook-Torrance GGX). Opt-in via `MaterialUsePBR()`. |

### 5.3 Texture Slots in the Shader

The pixel shader reads textures through a global `TexturePool`. Each texture receives a stable `uint32` index when registered. The material stores these indices as `albedoIndex`, `normalIndex`, `ormIndex`, `decalIndex`, and so on. The slot convention for `MaterialTexture` is: `0=Albedo`, `1=Normal`, `2=ORM`, `3=Decal`.

### 5.4 TexturePool

The `TexturePool` is a central SRV manager. `GetOrAdd(srv)` returns a stable index; if the SRV already exists, the existing index is returned. Default textures are created at engine startup: `WhiteTexture` (index 0), `FlatNormalTexture` (index 1), `DefaultORM` (index 2). Materials without an explicit texture assignment automatically receive these default values.

## 6. Render Pipeline

### 6.1 Two-Phase Rendering

Each frame goes through two main phases. Phase 1 is the shadow pass: the scene graph is rendered from the point of view of the directional light, and only depth information is written to the shadow map. Phase 2 is the normal pass: opaque geometry is sorted in the render queue by sort key (shader ID, material ID, depth) and rendered front-to-back. This is followed by the transparent pass with back-to-front sorting.

### 6.2 Render Queue and Sort Key

The render queue collects `RenderCommand` objects, each carrying a 64-bit sort key. The sort key encodes shader pointer (upper bits), material pointer, and depth so that state changes are minimized. For the transparent pass, pairs of `(depth, RenderCommand)` are used and sorted by descending depth so that more distant objects are rendered first.

### 6.3 Shadow Mapping

Shadow mapping uses a dedicated shadow map render target (`ShadowMapTarget`). Only one directional light can cast shadows at the same time. The shadow pass renders all shadow-casting meshes from the light's point of view into a depth texture. In the normal pass, this depth buffer is bound as an SRV and evaluated with PCF filtering (percentage closer filtering) for soft shadows. `MaterialReceiveShadows` controls at the material level whether an object receives shadows.

### 6.4 Render-to-Texture (RTT)

`CreateRenderTexture` creates a `RenderTextureTarget` instance. `SetRenderTarget` redirects all subsequent `RenderWorld()` calls into this texture. `GetRTTTexture` returns the resulting texture as `LPTEXTURE` for `MaterialSetAlbedo` or `EntityTexture`. `ResetRenderTarget` restores the backbuffer as the active render target. SRV hazards between the shadow pass and RTT are prevented by explicit unbinding before target switches.

### 6.5 Camera Layer Culling

Each entity carries a `LayerMask` (`uint32`). Each camera carries a `CullMask`. When building the render queue, only entities whose `LayerMask & CullMask != 0` are passed through. This makes separate render passes for UI, reflection, or other effects possible without creating scene copies. Constants such as `LAYER_DEFAULT`, `LAYER_ALL`, and custom bitmasks are defined in `RenderLayers.h`.

## 7. Shader System

### 7.1 Constant Buffer Assignment

Register assignment is fixed and must remain consistent across all shaders. Conflicts on one register are a silent error (no compile error, wrong rendering).

| **Register** | **Buffer / Content** |
|---|---|
| `b0 - MatrixBuffer` | World, view, and projection matrix of the current entity. Updated per mesh draw. |
| `b1 - LightBuffer` | Light data of the active directional light (color, direction, view/proj for shadow map). |
| `b2 - MaterialBuffer` | `MaterialData` of the current material (color, PBR parameters, flags, blend mode). |
| `b3+ - Custom Buffers` | Application-specific or feature-specific buffers. Reserved for custom shader extensions. |
| `b4 - BoneBuffer` | 128 bone matrices for skeletal animation. Updated each frame via `SetEntityBoneMatrices`. |

### 7.2 Vertex Format Flags

The shader compiler and input layout system work with `DWORD` flags from `CreateVertexFlags()`. Each flag activates one vertex stream: `D3DVERTEX_POSITION`, `D3DVERTEX_NORMAL`, `D3DVERTEX_COLOR`, `D3DVERTEX_TEX1`, `D3DVERTEX_TEX2`, `D3DVERTEX_TANGENT`, `D3DVERTEX_BONE_INDICES`, `D3DVERTEX_BONE_WEIGHTS`. `FillBuffer()` reads these flags and creates only the vertex buffers for which the shader actually expects data.

### 7.3 Known Limitations (DX11 / SM5.0)

Dynamic texture array indexing (`Texture2D gTex[16]`) is not compatible with SM5.0. The `TexturePool` therefore uses individual, explicitly named texture bindings. `UpdateSubresource()` is valid only for `DEFAULT` usage buffers; `DYNAMIC` buffers require `Map/Unmap`. Operator overloads for enums must be defined in header files, not in `.cpp` files, so they are visible in all translation units.

## 8. Skeletal Animation

The skinning system transfers up to four bone indices and four bone weights per vertex to the GPU. `VertexBoneData()` sets this data for a single vertex before `FillBuffer()` is called. `SetEntityBoneMatrices()` loads an array of `XMMATRIX` bone transformations into the bone buffer (`b4`) on the GPU. The skinning vertex shader transforms each vertex as a weighted sum of the four bone matrices. `hasSkinning` on the mesh controls which vertex shader path is used.

## 9. Timer and Frame Control

The timer is a singleton. `Core::BeginFrame()` and `Core::EndFrame()` bracket the update step and update the internal timing values. Access is exclusively through static getters.

| **Function** | **Meaning** |
|---|---|
| `Timer::GetDeltaTime()` | Elapsed time since the last frame in seconds (`double`). For frame-rate-independent movement. |
| `Timer::GetFPS()` | Currently measured frames per second. |
| `Timer::GetFixedStep()` | Fixed time increment for physics-relevant updates. |
| `Timer::GetFixedSteps()` | Number of fixed-update steps for the current frame. |

## 10. Known Architecture Patterns and Pitfalls

### 10.1 MeshAsset Co-Deletion

`DeleteMesh()` deletes the `MeshAsset` linked to the mesh by default. If two meshes share the same asset via `ShareMeshAsset()`, the asset of the mesh being deleted must be set to `nullptr` before calling `DeleteMesh()`. Otherwise the shared asset will be freed twice.

### 10.2 SRV Hazards Between Passes

DirectX 11 does not allow a texture to be bound as both a render target view and a shader resource view at the same time. Before every render target switch (shadow map -> backbuffer, backbuffer -> RTT), all SRV bindings must be explicitly set to `nullptr`. The `RenderManager` handles this internally, but custom backend extensions must observe this as well.

### 10.3 Memory Leak Through Container Ownership

When deleting resources, the resource must be removed from its owning container in the `ObjectManager`, not merely have its pointer set to null. The `ObjectManager` is the sole owner of all entities, materials, and assets. Pointers held in other structures are non-owning and must not be used for deletion.

### 10.4 F0 Computation in PBR

The F0 computation (Fresnel base reflectivity) for metallic materials must use the final blended albedo, not a pre-blend approximation. If F0 is calculated before detail map blending, visible artifacts occur with high metallic values combined with detail maps.

### 10.5 Debug Output

All `Debug::Log()` calls begin with the file name as the first element, followed by a descriptive message. Example: `Debug::Log("gdxdevice.cpp: Comparison Sampler created")`. This is the binding coding rule of the engine.

## 11. Planned Extensions

| **Feature** | **Status / Dependency** |
|---|---|
| Assimp model loader | In preparation. The scene graph foundation (`aiNode` mapping) is implemented. `vcpkg` integration is planned. |
| Octree spatial partitioning | Planned after RTT stabilization. For scene-level culling. |
| Per-mesh BVH (collision) | Planned after octree. For triangle-level collision detection. |
| Bullet physics integration | Preferred. A BlitzBasic-style wrapper API is planned. |
| SortKey revision (numeric IDs) | Highest-priority render optimization. Replaces pointer truncation with compact numeric IDs in a 64-bit bitfield. |
| Entity lifecycle (`OnStart`/`OnUpdate`/`OnDestroy`) | Self-registration in `ScriptManager` planned. |
| Constant buffer ring buffer | Low priority. Depends on profiling results. |
