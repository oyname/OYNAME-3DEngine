# OYNAME-3DEngine — Internal Architecture (DX11 · C++ · HLSL)

> Audience: engine developers (not end users).  
> Focus: internal structure, ownership, render pipeline, data flow.

---

## 1. Overview

The engine is organized into three layers:

1. **Public API (Wrapper)**
   - **File:** `include/gidx.h`
   - Static `Engine::…` functions (C-/C++-style facade).
   - Internally forwards to the singleton engine (`GDXEngine`) and its managers.

2. **Core / Runtime (Engine + Managers + Entities)**
   - **Examples:** `include/gdxengine.h`, `include/ObjectManager.h`, `include/RenderManager.h`, `include/Entity.h`
   - Responsible for object lifetime, scene data, render-queue building, update/render loop.

3. **Backend (DX11) + GPU Data**
   - **Examples:** `include/IRenderBackend.h`, `Dx11*GpuData.*`, `GDXDevice`, render targets
   - Encapsulates D3D11 device/context, render targets, constant buffers/SRV binding, shadow/main passes.

---

## 2. Central Hub: `GDXEngine`

- **File:** `include/gdxengine.h`
- Role: orchestration + access to managers and device layer.

### 2.1 Responsibilities
- Graphics/window initialization (via `GDXDevice` / `GDXInterface`)
- Global state (e.g., current camera, ambient)
- Top-level loop:
  - `UpdateWorld()`
  - `RenderWorld()`

### 2.2 Managers (typically stored as members)
- `ObjectManager` — ownership + creation of engine objects
- `RenderManager` — render pipeline + queues + backend
- `ShaderManager` — shader lifecycle
- `InputLayoutManager` — input layouts per VS signature/vertex format
- `BufferManager` — GPU buffers / constant buffers
- `TexturePool` — textures + SRV indexing

---

## 3. Scene Object Model

### 3.1 `Entity` (base class)
- **File:** `include/Entity.h`
- Typically contains:
  - Transform (world)
  - active/visible flags
  - type helpers (`IsMesh/IsLight/IsCamera`, `AsMesh/AsLight/AsCamera`)
- `LPENTITY` is an alias for `Entity*`.

### 3.2 `Mesh` (entity + render component)
- **File:** `include/Mesh.h`
- `Mesh` is the *instance* living in the scene (transform/visibility/etc.).
- Rendering data is attached through **`MeshRenderer`**.

### 3.3 `MeshAsset` (shareable geometry)
- **File:** `include/MeshAsset.h`
- Holds *only* geometry slots (submeshes) as references to `Surface*`.
- No transforms, no material overrides.

### 3.4 `Surface` (submesh + GPU buffers)
- **File:** `include/Surface.h` + GPU buffer structures
- Represents one draw unit:
  - vertex/index data
  - GPU buffers (currently DX11-near ownership)
  - optional legacy material pointer (compat)

### 3.5 `MeshRenderer` (slot material system)
- **File:** `include/MeshRenderer.h`
- Links:
  - `MeshAsset* asset` (geometry source)
  - `slotMaterials[i]` (per-slot material override)
  - `fallbackMaterial` (legacy fallback)

**Material resolution per slot `i`:**
1) `slotMaterials[i]` (override)  
2) `Surface::pMaterial` (legacy)  
3) `fallbackMaterial` (fallback)

---

## 4. Ownership & Lifetime (critical)

### 4.1 Owners
- **`ObjectManager`** owns (lifetime):
  - `Mesh`, `MeshAsset`, `Surface`, `Material`, `Shader`, `Light`, `Camera`, generic `Entity` objects
- **`RenderManager`** owns:
  - render targets (backbuffer/RTT/shadow)
  - render queues (frame lifetime)
  - backend instance (`IRenderBackend`)

### 4.2 Non-owning references
- `MeshRenderer.asset -> MeshAsset*` (owned by ObjectManager)
- `MeshAsset` stores `Surface*` slots (owned by ObjectManager)
- RenderQueue/RenderCommand store raw pointers (valid only within the frame!)

**Consequence:**  
Deletion/unregister must happen safely before the frame ends, otherwise commands may dereference freed memory.

---

## 5. Rendering Architecture

### 5.1 `RenderManager`
- **File:** `include/RenderManager.h`
- Responsibilities:
  - ensure backend (`EnsureBackend()`)
  - orchestrate shadow pass + main pass
  - build/sort/flush render queues
  - optional render-to-texture

### 5.2 `IRenderBackend` (decoupling layer)
- **File:** `include/IRenderBackend.h`
- Goal: reduce D3D11 detail inside `RenderManager`.
- Implementation: `Dx11RenderBackend`
- Typical duties:
  - begin/end passes (shadow/main/RTT)
  - bind constants (entity/light/shadow)
  - bind shadow SRV/sampler

### 5.3 RenderQueue & RenderCommand
- **Files:** `include/RenderQueue.h`, `include/RenderCommand.h`
- Design: flat list of draw commands:
  - opaque queue
  - transparent queue (sorted separately)

**Sorting:**
- Opaque: `SortKey()` (shader/material; currently pointer-based)
- Transparent: back-to-front by distance (correct blending order)

---

## 6. Render Pipeline (pass details)

### 6.1 Shadow Pass (Directional)
High-level:
1. bind shadow target, depth-only setup
2. compute light view/projection
3. update shadow-matrix constant buffer
4. iterate all renderable meshes:
   - active/visible?
   - `CastShadows`?
   - per slot/surface:
     - resolve material
     - check `material.castShadows`
     - bind VS-only / shadow shader variant
     - draw surface

### 6.2 Main Pass (Forward)
High-level:
1. bind backbuffer/viewport
2. update light constant buffer(s)
3. build render queue (one command per mesh slot)
4. sort + flush opaque queue (state batching)
5. flush transparent queue back-to-front

### 6.3 Render-to-Texture (optional)
- bind RTT target
- optionally switch to an RTT camera
- run main pass into RTT
- restore backbuffer

---

## 7. Material / Shader / Texture System

### 7.1 Material
- **Files:** `include/Material.h`, `Dx11MaterialGpuData.*`
- Typically contains:
  - PBR parameters (roughness/metallic/occlusion/baseColor/etc.)
  - texture indices (via TexturePool)
  - flags (ORM map, transparent, castShadows, ...)
  - pointer to render shader (`pRenderShader`)

### 7.2 Shader + InputLayout
- `ShaderManager` creates/manages VS/PS (+ variants)
- `InputLayoutManager` builds `ID3D11InputLayout` matching the VS signature

### 7.3 TexturePool
- loads textures, manages SRV handles/indices
- materials reference textures by indices, not SRV pointers

---

## 8. Data Flow (short diagram)

```text
gidx.h (Engine::...)
   |
   v
GDXEngine (Singleton)
 |   |   +--> ObjectManager (owns objects)
 |           |           +--> Mesh (Entity/Instance)
 |                   |                   +--> MeshRenderer
 |                           |                           +--> MeshAsset ----> [Surface* slots]
 |                           +--> slotMaterials[i] -> Material*
 |
 +--> RenderManager --> IRenderBackend --> Dx11RenderBackend --> D3D11
```

---

## 9. Typical Call Sequences

### 9.1 Linking mesh + material (conceptual)
1. `CreateMesh()` -> returns `LPENTITY` (mesh entity)
2. `CreateMeshAsset()` / `CreateSurface()` -> geometry slots
3. `MeshRenderer.asset = meshAsset`
4. `SetSlotMaterial(mesh, slot, mat)` or set fallback

### 9.2 Frame
1. `UpdateWorld()` (transforms/animation/logic)
2. `RenderWorld()`:
   - `RenderShadowPass()`
   - `RenderNormalPass()` (+ optional RTT)

---

## 10. Technical Notes / Known Weak Spots

- Opaque SortKey is pointer-based: works but is not deterministic across runs/ASLR.
  - Long term: ID-based sort keys (ShaderID/MaterialID/PSO key).
- Transparent: correct order beats batching; SRV caches help.
- RenderCommand uses raw pointers: queues must never outlive object memory.

---

**Entry files**
- API: `include/gidx.h`
- Engine: `include/gdxengine.h`
- Objects: `include/ObjectManager.h`
- Rendering: `include/RenderManager.h`, `include/IRenderBackend.h`
- Scene: `include/Entity.h`, `include/Mesh.h`, `include/MeshRenderer.h`, `include/MeshAsset.h`, `include/Surface.h`
