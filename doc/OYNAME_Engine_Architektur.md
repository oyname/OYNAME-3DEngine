# OYNAME-3DEngine --- Interne Architektur

DirectX 11 · C++ · HLSL

Dieses Dokument beschreibt die interne Struktur der
**OYNAME-3DEngine**.\
Es richtet sich an Entwickler, die an der Engine selbst arbeiten und die
Beziehungen zwischen **API, Managern, Backend-Klassen und DirectX‑11**
verstehen müssen.

Die Engine folgt einer klaren **3‑Schichten‑Architektur**:

Application │ ▼ Public API (gidx.h) │ ▼ Engine Layer (Manager + Scene) │
▼ Rendering Backend (DX11) │ ▼ DirectX 11

Die Schichten kommunizieren strikt **von oben nach unten**.\
Die Anwendung sieht **niemals interne Klassen oder DirectX‑Typen**.

------------------------------------------------------------------------

# 1. Schichtenmodell

  -----------------------------------------------------------------------
  Schicht                             Inhalt
  ----------------------------------- -----------------------------------
  Anwendungsschicht                   Nutzerseitiger Code (`main()`).
                                      Includet nur `gidx.h`. Keine
                                      Engine‑Interna sichtbar.

  Engine‑Schicht                      Manager und Szenenklassen
                                      (`ObjectManager`, `RenderManager`,
                                      `ShaderManager`, `TexturePool`,
                                      `BufferManager`,
                                      `InputLayoutManager`).

  Backend‑Schicht                     DirectX‑Implementierung
                                      (`GDXDevice`, `Dx11RenderBackend`,
                                      `SurfaceGpuBuffer`,
                                      `Dx11EntityGpuData`,
                                      `Dx11MaterialGpuData`,
                                      `Dx11ShadowMap`).
  -----------------------------------------------------------------------

------------------------------------------------------------------------

# 2. Modul‑Landkarte

## 2.1 Public API

Datei:

gidx.h

Eigenschaften:

-   einzige öffentliche Schnittstelle
-   BlitzBasic‑inspirierte Funktions‑API
-   keine DirectX‑Typen sichtbar
-   ruft intern Manager auf

Beispiele:

``` cpp
Engine::Graphics(1280,720);
Engine::CreateMesh(&mesh, mat);
Engine::RenderWorld();
```

Die API enthält **keine Engine‑Logik**, sondern nur Delegation.

------------------------------------------------------------------------

# 3. Entity‑System

Alle Szenenobjekte basieren auf der Basisklasse **Entity**.

Entity ├─ Transform ├─ Parent ├─ Children └─ LayerMask

## Transform

Jede Entity besitzt:

-   Position
-   Rotation
-   Skalierung

Weltmatrix:

Local Space: world = local \* parentWorld

World Space: world = local

------------------------------------------------------------------------

# 4. Mesh‑ und Surface‑System

Die Engine verwendet eine **Mesh‑Surface‑Architektur**.

Mesh ├─ Surface 0 ├─ Surface 1 └─ Surface 2

Eine Surface entspricht einem **Submesh**.

### Geometrieaufbau

``` cpp
AddVertex()
VertexNormal()
VertexTexCoord()
AddTriangle()
FillBuffer()
```

`FillBuffer()` überträgt die Daten in GPU‑Buffer.

------------------------------------------------------------------------

# 5. Rendering‑Pipeline

Der Frame‑Ablauf:

UpdateWorld()\
RenderWorld()\
Flip()

RenderWorld:

-   RenderShadowPass()
-   BuildRenderQueue()
-   FlushOpaqueQueue()
-   FlushTransparentQueue()

------------------------------------------------------------------------

# 6. RenderQueue

RenderCommand:

-   Mesh
-   Surface
-   Material
-   Shader
-   WorldMatrix
-   Backend

Jeder Command entspricht **einem Draw‑Call**.

------------------------------------------------------------------------

# 7. SortKey

Opaque‑Commands werden nach folgendem Key sortiert:

uint64 SortKey = ShaderPointer \<\< 32 \| MaterialPointer

``` cpp
uint64_t SortKey() const noexcept
{
    uint64_t sh = (reinterpret_cast<uintptr_t>(shader) & 0xFFFFFFFF);
    uint64_t mt = (reinterpret_cast<uintptr_t>(material) & 0xFFFFFFFF);
    return (sh << 32) | mt;
}
```

Ziel: Minimierung von Shader‑ und Material‑State‑Wechseln.

------------------------------------------------------------------------

# 8. Transparent‑Pass

Transparente Surfaces werden getrennt sortiert:

Container:

m_transFrame (depth, RenderCommand)

Sortierung: **back‑to‑front**

BlendState:

SrcBlend = SRC_ALPHA\
DestBlend = INV_SRC_ALPHA

------------------------------------------------------------------------

# 9. SRV‑Cache

Der RenderManager speichert die zuletzt gebundenen ShaderResourceViews.

Vor jedem `PSSetShaderResources()` wird geprüft, ob sich der SRV‑Bind
geändert hat.\
Nur bei Änderung erfolgt ein neuer DirectX‑Call.

------------------------------------------------------------------------

# 10. Material‑System

Material‑Parameter werden im ConstantBuffer **b2** gespeichert.

``` hlsl
cbuffer MaterialBuffer : register(b2)
{
    float4 gBaseColor;
    float4 gSpecularColor;
    float4 gEmissiveColor;
    float4 gUvTilingOffset;

    float4 gPbr;
    float4 gAlpha;

    uint4  gTexIndex;
    uint4  gMisc;
};
```

Der C++‑Struct muss exakt dieses Layout besitzen.

------------------------------------------------------------------------

# 11. Shadow Mapping

Shadow Maps werden aus Sicht des Directional Lights gerendert.

RenderShadowPass:

-   Shadow Depth Target
-   Vertex Shader

Im Hauptpass:

t16 = Shadow Map\
s7 = Comparison Sampler

------------------------------------------------------------------------

# 12. Vertex‑Format‑Bitmask

  Flag                 HLSL
  -------------------- -----------
  D3DVERTEX_POSITION   POSITION
  D3DVERTEX_NORMAL     NORMAL
  D3DVERTEX_TANGENT    TANGENT
  D3DVERTEX_COLOR      COLOR
  D3DVERTEX_TEX1       TEXCOORD0
  D3DVERTEX_TEX2       TEXCOORD1

Der `InputLayoutManager` erzeugt daraus die DX11‑Layoutbeschreibung.

------------------------------------------------------------------------

# 13. Abhängigkeitsregeln

Application\
- darf nur `gidx.h` includen

API\
- darf keine DirectX‑Header includen

Engine‑Layer\
- keine `d3d11.h` in öffentlichen Headern

Backend\
- enthält alle DirectX‑Objekte
