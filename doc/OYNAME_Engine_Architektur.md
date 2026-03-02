# OYNAME-3DEngine — Interne Architektur

DirectX 11 · C++ · HLSL

---

## 1. Überblick

Dieses Dokument beschreibt die interne Implementierung der OYNAME-3DEngine. Es richtet sich an Entwickler die an der Engine selbst arbeiten und die Zusammenhänge zwischen Managern, Backend-Klassen, HLSL-Shadern und dem DirectX-11-API verstehen müssen.

Die Engine ist in drei horizontale Schichten gegliedert. Die öffentliche API in `gidx.h` ist die einzige Schnittstelle nach außen. Die mittlere Schicht enthält die Manager-Logik. Das Backend kapselt alle DirectX-Aufrufe.

---

## 2. Schichtenmodell

Die drei Schichten kommunizieren strikt von oben nach unten. Anwendungscode sieht niemals interne Klassen oder DirectX-Typen.

| Schicht | Inhalt |
|---|---|
| Anwendungsschicht | Nutzerseitiger Code in `main()`. Includet nur `gidx.h`. Kein DirectX, keine internen Typen sichtbar. |
| Engine-Schicht (Manager) | ObjectManager, RenderManager, ShaderManager, LightManager, TexturePool, BufferManager, InputLayoutManager. |
| Backend-Schicht (DX11) | GDXDevice, Dx11RenderBackend, SurfaceGpuBuffer, Dx11EntityGpuData, Dx11MaterialGpuData, Dx11ShadowMap. |

---

## 3. Manager-System

Jeder Manager hat eine klar abgegrenzte Verantwortung. Manager besitzen keine Querverweise untereinander — sie erhalten benötigte Referenzen per Dependency Injection im Konstruktor oder als Parameter.

| Manager | Verantwortung |
|---|---|
| `ObjectManager` | Erzeugt und verwaltet den Lebenszyklus aller Engine-Objekte (Mesh, Camera, Light, Shader, Material, Surface). Einziger Besitzer der Objekt-Listen. |
| `RenderManager` | Koordiniert Shadow-Pass, Opaque-Pass und Transparent-Pass. Verwaltet RenderTargets, BlendStates, SRV-Cache und RTT-Support. |
| `ShaderManager` | Kompiliert HLSL per D3DCompileFromFile, erstellt VS/PS-Objekte, verwaltet Standard-Shader. |
| `TexturePool` | Globaler SRV-Pool mit stabilen Indices (0..N). Dedupliziert Texturen automatisch. Liefert White/FlatNormal/ORM als Fallback-Indices. |
| `BufferManager` | Erstellt und aktualisiert D3D11-Buffer: VertexBuffer, IndexBuffer, ConstantBuffer. |
| `InputLayoutManager` | Erstellt `D3D11InputLayout` aus Vertex-Format-Bitmask. Mappt `D3DVERTEX_*`-Flags auf `D3D11_INPUT_ELEMENT_DESC`. |

---

## 4. Entity-System

Alle Szenenobjekte erben von der gemeinsamen Basisklasse `Entity`. Mesh, Camera und Light sind gleichwertige Entitäten mit identischer Transform-Schnittstelle. Das eliminiert typspezifische Sonderfälle in der API.

### 4.1 Transform und Weltmatrix

Jede Entity besitzt ein Transform-Objekt mit Position, Rotation und Skalierung. `GetWorldMatrix()` berücksichtigt die Parent-Child-Hierarchie rekursiv:

```cpp
// Space::Local (Standard):  worldMatrix = localMatrix * parent->GetWorldMatrix()
// Space::World:             worldMatrix = localMatrix  (Parent ignoriert)
```

### 4.2 Parent-Child-Hierarchie

`SetEntityParent()` verknüpft zwei Entities. Die Weltmatrix des Kindes ergibt sich automatisch aus der Multiplikation seiner lokalen Matrix mit der Weltmatrix des Elternobjekts. `DetachEntity()` löst die Verknüpfung. Hierarchien beliebiger Tiefe werden unterstützt.

---

## 5. Rendering-Pipeline

Der Render-Durchlauf läuft pro Frame in folgender Reihenfolge ab:

| Phase | Beschreibung |
|---|---|
| `UpdateWorld()` | Transformationen aller Entities berechnen, Kamera-Matrizen aktualisieren, Licht-Buffer vorbereiten. |
| `RenderShadowPass()` | Shadow Map aus Directional-Light-Perspektive rendern. VS-only, kein Pixel-Shader. PCF-Filterung in 3×3-Kernel. |
| `BuildRenderQueue()` | Alle aktiven Meshes durchlaufen. Surfaces nach `Material::IsTransparent()` in `m_opaque` oder `m_transFrame` einsortieren. |
| `FlushRenderQueue()` | `m_opaque` nach SortKey (Shader-Ptr << 32 \| Material-Ptr) sortiert abarbeiten. State-Wechsel nur bei Shader- oder Material-Änderung. |
| `FlushTransparentQueue()` | `m_transFrame` back-to-front nach Kameraabstand (LengthSq) sortieren, AlphaBlendState setzen, zeichnen, BlendState zurücksetzen. |
| `Flip()` | Present — fertiges Bild auf den Monitor ausgeben. |

### 5.1 RenderQueue und SortKey

Jeder `RenderCommand` ist ein eigenständiger, selbstausführbarer Draw-Call. Er trägt Mesh, Surface, Weltmatrix, Shader, Material und Backend-Referenz. Der SortKey kombiniert Shader- und Material-Adresse zu einem `uint64`:

```cpp
uint64_t SortKey() const noexcept
{
    uint64_t sh = (reinterpret_cast<uintptr_t>(shader)   & 0xFFFFFFFF);
    uint64_t mt = (reinterpret_cast<uintptr_t>(material) & 0xFFFFFFFF);
    return (sh << 32) | mt;
}
```

Durch Sortierung nach diesem Key werden alle Draw-Calls mit demselben Shader aufeinanderfolgend ausgeführt, dann innerhalb desselben Shaders nach Material. Das minimiert GPU-State-Wechsel ohne echtes GPU-Instancing (DrawIndexedInstanced).

### 5.2 Transparent-Pass

Surfaces deren Material `MF_TRANSPARENT` gesetzt hat, landen in `m_transFrame` als Paar `(float depth, RenderCommand)`. Depth ist LengthSq vom Kamera-Ursprung zum Mesh-Ursprung — ausreichend für korrekte Back-to-Front-Sortierung ohne Wurzelberechnung.

Vor `FlushTransparentQueue()` wird der Alpha-BlendState gebunden. Nach dem Durchlauf wird der No-BlendState wiederhergestellt. Der SRV-Cache wird invalidiert weil sich der GPU-Zustand geändert hat.

```cpp
// BlendState-Konfiguration (EnsureBackend):
D3D11_BLEND_DESC bd{};
bd.RenderTarget[0].BlendEnable    = TRUE;
bd.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
bd.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
bd.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
bd.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
bd.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
```

---

## 6. Material- & Shader-System

### 6.1 Texture-Slots im Shader

Der Standard-Pixel-Shader belegt folgende SRV-Slots. Die Slots sind fest verdrahtet — SM5.0 erlaubt kein dynamisches Array-Indexing in Pixel-Shadern.

| Slot | Name | Beschreibung |
|---|---|---|
| t0 | `gAlbedo` | Albedo-Textur. Fallback: `WhiteIndex()`. |
| t1 | `gNormalMap` | Normal Map. Nur ausgewertet wenn `MF_USE_NORMAL_MAP` gesetzt. Fallback: `FlatNormalIndex()`. |
| t2 | `gORM` | ORM-Map (Occlusion/Roughness/Metallic). Nur wenn `MF_USE_ORM_MAP`. |
| t3 | `gDecal` | Zweite Textur für Blend-Modi. Intern als `decalIndex` verwaltet. Fallback: White. |
| t16 | `shadowMapTexture` | Shadow Map. Comparison Sampler s7. |

### 6.2 MaterialBuffer (b2) — Constant-Buffer-Layout

Der Material-Constant-Buffer ist 128 Bytes groß und muss exakt mit der HLSL-`cbuffer`-Definition übereinstimmen (`static_assert` im Code sichert das ab).

```cpp
struct alignas(16) MaterialCB   // 128 Bytes
{
    XMFLOAT4 baseColor;         // Diffuse-Farbe + Alpha
    XMFLOAT4 specularColor;
    XMFLOAT4 emissiveColor;
    XMFLOAT4 uvTilingOffset;    // xy=tiling  zw=offset

    XMFLOAT4 pbr;              // x=metallic y=roughness z=normalScale w=occlusion
    XMFLOAT4 alpha;            // x=shininess y=transparency z=alphaCutoff w=receiveShadows

    XMUINT4  texIndex;         // x=albedo y=normal z=orm w=decal (Pool-Indices)
    XMUINT4  misc;             // x=blendMode y=materialFlags z=blendFactor(bits) w=unused
};
```

`gMisc.z`: blendFactor wird als float-Bits per `memcpy` in einen `uint32` übertragen und im Shader per `asfloat()` wieder gelesen. Das vermeidet eine Erweiterung des Constant-Buffers über 128 Bytes.

### 6.3 MaterialFlags

Die `materialFlags` in `gMisc.y` sind ein Bitmask der im Pixel-Shader per `#define` ausgewertet wird:

| Flag | Wert | Bedeutung |
|---|---|---|
| `MF_ALPHA_TEST` | 1<<0 | Alpha-Test per `discard`. Läuft vor der Blend-Modi-Berechnung. |
| `MF_DOUBLE_SIDED` | 1<<1 | Reserviert. Rasterizer-State muss extern gesetzt werden. |
| `MF_UNLIT` | 1<<2 | Lighting überspringen. Albedo direkt ausgeben. |
| `MF_USE_NORMAL_MAP` | 1<<3 | Normal-Map-Berechnung im PS aktivieren. Ohne dieses Flag wird die geometrische Normale verwendet. |
| `MF_USE_ORM_MAP` | 1<<4 | ORM-Sampling aktivieren. |
| `MF_USE_EMISSIVE` | 1<<5 | Emissive-Farbe zur finalen Farbe addieren. |
| `MF_TRANSPARENT` | 1<<6 | Surface wird in den Transparent-Pass geroutet (`BuildRenderQueue`). |

### 6.4 Blend-Modi (t3 / gDecal)

Wenn `blendMode > 0` sampelt der Shader `gDecal` auf `uv1` (texCoord2) und mischt das Ergebnis mit der Albedo-Farbe. `blendFactor` skaliert den Einfluss von 0.0 bis 1.0.

| Mode | Name | HLSL-Formel |
|---|---|---|
| 0 | off | Kein Blend. gDecal wird nicht gesampelt. |
| 1 | Multiply | `lerp(albedo, albedo * tex2, f)` — Abdunkeln. |
| 2 | Multiply×2 | `lerp(albedo, albedo * tex2 * 2, f)` — Detail-Map. |
| 3 | Additive | `lerp(albedo, albedo + tex2, f)` — Glühen, Feuer. |
| 4 | Lerp/Alpha | `lerp(albedo, tex2.rgb, tex2.a * f)` — Decals. |
| 5 | Luminanz | `lum = dot(tex2, float3(0.2126, 0.7152, 0.0722)); lerp(albedo, albedo + tex2*lum, f)` |

---

## 7. GpuData-Pattern

Szene-Klassen (Entity, Material, Surface) enthalten keine DirectX-Typen. Alle GPU-Ressourcen werden in separaten GpuData-Objekten gekapselt, die als Pointer gehalten werden. Das entkoppelt Scene-Layer-Header von DirectX-Includes.

| Klasse | Inhalt |
|---|---|
| `Dx11EntityGpuData` | Weltmatrix-Constant-Buffer (b0) pro Entity. Wird in `Entity::Update()` aktualisiert. |
| `Dx11MaterialGpuData` | Material-Constant-Buffer (b2) + Texture-SRVs + Sampler-States. `UpdateConstantBuffer()` mappt und kopiert per `D3D11_MAP_WRITE_DISCARD`. |
| `SurfaceGpuBuffer` | VertexBuffer + IndexBuffer pro Surface. `FillBuffer()` erstellt, `UpdateVertexBuffer()` aktualisiert partiell. |

---

## 8. SRV-Cache im RenderManager

Der RenderManager hält ein Array von vier `ID3D11ShaderResourceView`-Pointern als Cache (`m_boundSRVs[4]`). Vor jedem `PSSetShaderResources(0, 4, srvs)` wird per `memcmp` geprüft ob sich die SRVs gegenüber dem letzten Bind geändert haben. Nur bei Änderung wird der DX11-Call ausgeführt. Nach `FlushTransparentQueue()` wird der Cache mit `memset(m_boundSRVs, 0)` invalidiert.

---

## 9. Shadow Mapping

Shadow Mapping arbeitet mit einem dedizierten `ShadowMapTarget` das aus der Perspektive des Directional Lights gerendert wird. Der Shadow-Pass bindet nur den Vertex-Shader (`VS_ONLY`), der Pixel-Shader bleibt ungebunden.

Im Haupt-Pass wird die Shadow Map auf Slot t16 gebunden und per `SampleCmpLevelZero` mit dem Comparison-Sampler (s7) ausgewertet. PCF-Filterung mittelt 3×3 Samples mit adaptivem Bias basierend auf NdotL.

```hlsl
// Bias-Berechnung im PS (verhindert Shadow-Acne):
float ndotl = saturate(dot(normalize(normal), normalize(-lightDir)));
float bias  = max(0.0005f, 0.0030f * (1.0f - ndotl));
```

---

## 10. Vertex-Format-Bitmask

`CreateShader()` und `CreateInputLayout()` arbeiten mit einem Bitmask der die im Vertex-Buffer enthaltenen Attribute beschreibt. `InputLayoutManager::CreateInputLayoutVertex()` erzeugt daraus die `D3D11_INPUT_ELEMENT_DESC`-Liste und validiert sie gegen den VS-Bytecode.

| Flag | HLSL-Semantik |
|---|---|
| `D3DVERTEX_POSITION` | `float3 POSITION` — Pflichtfeld. |
| `D3DVERTEX_NORMAL` | `float3 NORMAL` |
| `D3DVERTEX_TANGENT` | `float4 TANGENT` (xyz + Handedness w). Pflicht für Normal-Map-Shader. |
| `D3DVERTEX_COLOR` | `float4 COLOR` |
| `D3DVERTEX_TEX1` | `float2 TEXCOORD0` — Albedo-UV |
| `D3DVERTEX_TEX2` | `float2 TEXCOORD1` — Lightmap/Detail-UV (für Blend-Modi) |
| `D3DVERTEX_BONE_INDICES` | `uint4 BLENDINDICES` — Skelettanimation |
| `D3DVERTEX_BONE_WEIGHTS` | `float4 BLENDWEIGHT` — Skelettanimation |

Fehlt ein vom Shader erwartetes Attribut im Bitmask, schlägt `CreateInputLayout()` mit `E_INVALIDARG` fehl. Der Standard-Shader erwartet alle sechs Basis-Attribute inkl. TANGENT.

---

*OYNAME-3DEngine · Internes Architekturdokument · DirectX 11 / C++ / HLSL*
