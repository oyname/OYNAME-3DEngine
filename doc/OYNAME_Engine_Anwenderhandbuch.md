# OYNAME-3DEngine — Anwenderhandbuch

DirectX 11 · C++ · BlitzBasic-inspirierte API

---

## 1. Einstieg

Die OYNAME-3DEngine stellt eine einfache, BlitzBasic-inspirierte API bereit. Das gesamte Include beschränkt sich auf eine einzige Datei:

```cpp
#include "gidx.h"
```

Kein DirectX, keine Handles, keine Verwaltungsarbeit. Alle Engine-Funktionen liegen im Namespace `Engine::`, Hilfsfunktionen für das Fenster unter `Windows::` und Systemfunktionen unter `Core::`.

### 1.1 Minimales Programm

```cpp
int main()
{
    Engine::Graphics(1280, 720);      // Fenster öffnen, DirectX initialisieren

    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 0.0f, -5.0f);

    while (Windows::MainLoop())
    {
        Engine::Cls(0, 64, 128);      // Backbuffer leeren
        Engine::UpdateWorld();         // Transformationen
        Engine::RenderWorld();         // Rendern
        Engine::Flip();               // Bild zeigen
    }
    return 0;
}
```

---

## 2. API-Referenz

### 2.1 Engine-Start

| Funktion | Beschreibung |
|---|---|
| `Graphics(w, h)` | Fenster öffnen und DirectX 11 initialisieren. Muss als erstes aufgerufen werden. |
| `Cls(r, g, b)` | Backbuffer leeren (0..255 pro Kanal). |
| `UpdateWorld()` | Transformationen, Kamera-Matrizen und Licht-Buffer aktualisieren. |
| `RenderWorld()` | Shadow-Pass, Opaque-Pass und Transparent-Pass ausführen. |
| `Flip()` | Fertiges Bild auf dem Monitor anzeigen (Present). |
| `SetVSync(bool)` | Vertikale Synchronisation ein- oder ausschalten. |

### 2.2 Entity-Transformation

`PositionEntity`, `RotateEntity`, `TurnEntity`, `MoveEntity` und `ScaleEntity` funktionieren auf allen Entity-Typen gleich — Mesh, Camera und Light. Das optionale Space-Flag (`Space::Local` / `Space::World`) steuert das Koordinatensystem.

| Funktion | Beschreibung |
|---|---|
| `PositionEntity(e, x,y,z)` | Absolute Position setzen. |
| `MoveEntity(e, x,y,z)` | Relativ zur aktuellen Position bewegen (im lokalen Raum). |
| `RotateEntity(e, p,y,r)` | Absolute Rotation setzen (Pitch, Yaw, Roll in Grad). |
| `TurnEntity(e, p,y,r)` | Rotation relativ zur aktuellen Rotation hinzufügen. |
| `ScaleEntity(e, x,y,z)` | Skalierung setzen. |
| `LookAt(e, target)` | Entity auf ein Ziel ausrichten. |
| `EntityX/Y/Z(e)` | Weltposition abfragen. |
| `EntityLayer(e, mask)` | Layer-Bitmask setzen (für Camera-Cull-Masks). |
| `SetEntityParent(e, parent)` | Elternobjekt setzen. Transformationen werden vererbt. |
| `DetachEntity(e)` | Vom Elternobjekt lösen. |
| `ShowEntity(e, bool)` | Sichtbarkeit ein-/ausschalten. |

### 2.3 Kamera

| Funktion | Beschreibung |
|---|---|
| `CreateCamera(&cam)` | Kamera erstellen. |
| `SetCamera(cam)` | Aktive Kamera setzen. |
| `CameraCullMask(cam, mask)` | Bestimmt welche Layer diese Kamera rendert (Bitmask). |

### 2.4 Licht

| Funktion | Beschreibung |
|---|---|
| `CreateLight(&light, type)` | Licht erstellen. type: `D3DLIGHT_DIRECTIONAL` oder `D3DLIGHT_POINT`. |
| `LightColor(light, r,g,b)` | Lichtfarbe setzen (0.0..1.0). |
| `SetDirectionalLight(light)` | Licht als Schatten werfendes Directional-Light registrieren. |
| `SetAmbientColor(r,g,b)` | Globale Ambient-Farbe setzen. |
| `EntityCastShadows(e, bool)` | Steuert ob ein Mesh Schatten wirft. |

### 2.5 Mesh & Surface

Ein Mesh besteht aus einer oder mehreren Surfaces. Jede Surface hat ihre eigene Geometrie und ihr eigenes Material. Vertices werden Surface-lokal hinzugefügt; der Index entspricht der Reihenfolge der `AddVertex`-Aufrufe.

| Funktion | Beschreibung |
|---|---|
| `CreateMesh(&mesh, mat)` | Leeres Mesh erstellen, optional mit Material. |
| `CreateSurface(&surf, mesh)` | Neue Surface am Mesh anlegen. |
| `AddVertex(surf, x,y,z)` | Vertex hinzufügen. |
| `VertexNormal(surf, x,y,z)` | Normale für den zuletzt hinzugefügten Vertex. |
| `VertexColor(surf, r,g,b)` | Farbe für den zuletzt hinzugefügten Vertex (0..255). |
| `VertexTexCoord(surf, u,v)` | UV für Slot 0 (Albedo). |
| `VertexTexCoord2(surf, u,v)` | UV für Slot 1 (zweite Textur / Blend). |
| `AddTriangle(surf, a,b,c)` | Dreieck aus drei Vertex-Indices. |
| `FillBuffer(surf)` | Geometrie nach dem Aufbau auf die GPU übertragen. Einmalig aufrufen. |
| `SurfaceMaterial(surf, mat)` | Material einer einzelnen Surface zuweisen. |

### 2.6 Shader

Normalerweise wird der Standard-Shader automatisch verwendet. Eigene Shader werden mit `CreateShader()` geladen. Der Vertex-Format-Bitmask muss exakt den Attributen entsprechen die im Vertex-Shader als `VS_INPUT` erwartet werden.

| Funktion | Beschreibung |
|---|---|
| `CreateShader(&sh, vsFile, vsEntry, psFile, psEntry, flags)` | HLSL-Shader kompilieren und Input-Layout erstellen. |
| `CreateVertexFlags(pos,norm,col,uv1,uv2,tangent)` | Bitmask aus booleschen Flags aufbauen. |

Der Standard-Shader erwartet alle sechs Basis-Attribute (Position, Normal, Tangent, Color, UV1, UV2). Fehlt ein Attribut, schlägt `CreateShader()` mit `E_INVALIDARG` fehl.

### 2.7 Material

| Funktion | Beschreibung |
|---|---|
| `CreateMaterial(&mat)` | Material mit Standard-Shader erstellen. |
| `CreateMaterial(&mat, shader)` | Material mit eigenem Shader erstellen. |
| `MaterialTexture(mat, tex, slot)` | Textur zuweisen: 0=Albedo, 1=zweite Textur, 2=Normal, 3=ORM. |
| `MaterialSetAlbedo(mat, tex)` | Albedo-Textur (entspricht Slot 0). |
| `MaterialSetNormal(mat, tex)` | Normal Map (entspricht Slot 2). Aktiviert `MF_USE_NORMAL_MAP`. |
| `MaterialSetORM(mat, tex)` | ORM-Textur (entspricht Slot 3). Aktiviert `MF_USE_ORM_MAP`. |
| `MaterialColor(mat, r,g,b,a)` | Diffuse-Grundfarbe und Alpha-Skalar (0.0..1.0). |
| `MaterialMetallic(mat, v)` | PBR-Metallic (0.0..1.0). |
| `MaterialRoughness(mat, v)` | PBR-Roughness (0.0..1.0). |
| `MaterialNormalScale(mat, v)` | Stärke der Normal Map. |
| `MaterialEmissiveColor(mat, r,g,b)` | Emissive-Farbe. Aktiviert `MF_USE_EMISSIVE`. |
| `MaterialBlendMode(mat, mode)` | Blend-Modus für Slot-1-Textur: 0=off 1=Multiply 2=Mul×2 3=Add 4=Lerp 5=Luminanz. |
| `MaterialBlendFactor(mat, v)` | Mischstärke der zweiten Textur (0.0..1.0). |
| `MaterialAlphaTest(mat, bool)` | Alpha-Test aktivieren. Pixel unter `alphaCutoff` werden verworfen. |
| `MaterialAlphaCutoff(mat, v)` | Schwellwert für Alpha-Test (Standard: 0.5). |
| `MaterialTransparent(mat, bool)` | Echtes Alpha-Blending aktivieren. Material kommt in den Transparent-Pass. |
| `MaterialReceiveShadows(mat, bool)` | Shadow Map auswerten. |
| `MaterialShininess(mat, v)` | Specular-Schärfe. |
| `MaterialUVTilingOffset(mat, sx,sy,ox,oy)` | UV-Skalierung und Verschiebung. |
| `EntityMaterial(e, mat)` | Material einem Mesh zuweisen. |

### 2.8 Textur

| Funktion | Beschreibung |
|---|---|
| `LoadTexture(&tex, filename)` | Textur aus Datei laden (BMP, PNG, JPG, DDS …). PNG mit Alpha-Kanal wird unterstützt. |
| `CreateTexture(&tex, w, h)` | Leere Textur erstellen (für programmatische Füllung). |
| `LockBuffer(tex)` | Textur zum Schreiben sperren. |
| `UnlockBuffer(tex)` | Textur entsperren und GPU aktualisieren. |
| `SetPixel(tex, x, y, r,g,b,a)` | Einzelnen Pixel setzen (nach `LockBuffer`). |

### 2.9 Render-to-Texture

| Funktion | Beschreibung |
|---|---|
| `CreateRenderTexture(&rtt, w, h)` | Render-Ziel erstellen. |
| `SetRenderTarget(rtt, cam)` | In dieses Render-Ziel rendern. |
| `ResetRenderTarget()` | Zurück zum normalen Backbuffer. |
| `GetRTTTexture(rtt)` | Ergebnis-Textur abfragen (als Materialtextur verwendbar). |
| `SetRTTClearColor(rtt, r,g,b,a)` | Hintergrundfarbe des RTT-Passes. |

### 2.10 Layer-System

| Funktion | Beschreibung |
|---|---|
| `EntityLayer(e, mask)` | Layer-Bitmask einer Entity (Standard: `LAYER_ALL = 0xFFFFFFFF`). |
| `CameraCullMask(cam, mask)` | Kamera rendert nur Entities deren LayerMask & CullMask != 0. |

Das Layer-System eignet sich für selektives Rendern: HUD auf einem separaten Layer, Minimap-Kamera mit anderem CullMask, RTT-Kamera die nur bestimmte Objekte sieht.

---

## 3. Beispiele

### 3.1 Geometrie aufbauen

```cpp
LPENTITY mesh = nullptr;
Engine::CreateMesh(&mesh, nullptr);

LPSURFACE surf = nullptr;
Engine::CreateSurface(&surf, mesh);

Engine::AddVertex(surf, -1.0f, -1.0f, 0.0f);
Engine::VertexNormal(surf, 0, 0, -1);
Engine::VertexColor(surf, 255, 255, 255);
Engine::VertexTexCoord(surf, 0.0f, 1.0f);

// ... weitere Vertices ...

Engine::AddTriangle(surf, 0, 1, 2);
Engine::FillBuffer(surf);
```

### 3.2 PBR-Material

```cpp
LPTEXTURE albedo = nullptr, normal = nullptr, orm = nullptr;
Engine::LoadTexture(&albedo, L"..\\media\\stein_albedo.png");
Engine::LoadTexture(&normal, L"..\\media\\stein_normal.png");
Engine::LoadTexture(&orm,    L"..\\media\\stein_orm.png");

LPMATERIAL mat = nullptr;
Engine::CreateMaterial(&mat);
Engine::MaterialSetAlbedo(mat, albedo);
Engine::MaterialSetNormal(mat, normal);
Engine::MaterialSetORM(mat, orm);
Engine::MaterialMetallic(mat, 0.0f);
Engine::MaterialRoughness(mat, 0.8f);
Engine::MaterialNormalScale(mat, 1.2f);
Engine::MaterialReceiveShadows(mat, true);
```

### 3.3 Alpha-Test (Bitmask)

Geeignet für Bäume, Zäune, Gitter. Kein Blending, keine Sortierung — performant und mit harten Kanten.

```cpp
LPTEXTURE texBaum = nullptr;
Engine::LoadTexture(&texBaum, L"..\\media\\baum.png");  // PNG mit Alpha

LPMATERIAL mat = nullptr;
Engine::CreateMaterial(&mat);
Engine::MaterialTexture(mat, texBaum, 0);
Engine::MaterialAlphaCutoff(mat, 0.5f);
Engine::MaterialAlphaTest(mat, true);
```

### 3.4 Alpha-Blending

Für Glas, Partikel, halbtransparente Flächen. Das Material wird automatisch in den Transparent-Pass verschoben und back-to-front sortiert gerendert. Der Alpha-Kanal der Textur bestimmt die Transparenz pixelgenau.

```cpp
LPTEXTURE texGlas = nullptr;
Engine::LoadTexture(&texGlas, L"..\\media\\glas.png");

LPMATERIAL mat = nullptr;
Engine::CreateMaterial(&mat);
Engine::MaterialTexture(mat, texGlas, 0);
Engine::MaterialTransparent(mat, true);

// Optional: globalen Alpha-Wert skalieren
Engine::MaterialColor(mat, 1.0f, 1.0f, 1.0f, 0.7f);  // 70% opak
```

### 3.5 Multi-Textur

Slot 0 ist die Albedo-Textur, Slot 1 die zweite Textur. `BlendMode` und `BlendFactor` steuern wie beide Texturen kombiniert werden. Für Multi-Textur müssen im Shader-Bitmask `D3DVERTEX_TEX1` und `D3DVERTEX_TEX2` gesetzt sein und die Vertices brauchen beide UV-Slots.

```cpp
LPTEXTURE texBrick  = nullptr;
LPTEXTURE texDetail = nullptr;
Engine::LoadTexture(&texBrick,  L"..\\media\\brick.png");
Engine::LoadTexture(&texDetail, L"..\\media\\detail.png");

LPMATERIAL mat = nullptr;
Engine::CreateMaterial(&mat, multiTexShader);
Engine::MaterialTexture(mat, texBrick,  0);  // Albedo
Engine::MaterialTexture(mat, texDetail, 1);  // zweite Textur

// Blend-Modi:
// 0 = off          nur Slot 0
// 1 = Multiply     Abdunkeln (Lightmap, Schatten)
// 2 = Multiply×2   Detail-Map
// 3 = Additive     Glühen, Feuer
// 4 = Lerp/Alpha   Decals per Alpha-Kanal
// 5 = Luminanz     Overlay mit schwarzem Hintergrund
Engine::MaterialBlendMode(mat, 1);
Engine::MaterialBlendFactor(mat, 0.5f);  // 50% Einfluss

// Shader-Bitmask für Multi-Textur:
// D3DVERTEX_POSITION | D3DVERTEX_NORMAL | D3DVERTEX_TANGENT
//   | D3DVERTEX_COLOR | D3DVERTEX_TEX1 | D3DVERTEX_TEX2
```

### 3.6 Licht und Schatten

```cpp
LPENTITY dirLight = nullptr;
Engine::CreateLight(&dirLight, D3DLIGHT_DIRECTIONAL);
Engine::LightColor(dirLight, 1.0f, 1.0f, 1.0f);
Engine::TurnEntity(dirLight, 45.0f, 0.0f, 0.0f);
Engine::SetDirectionalLight(dirLight);

Engine::SetAmbientColor(0.15f, 0.15f, 0.2f);

Engine::EntityCastShadows(mesh, true);
Engine::MaterialReceiveShadows(mat, true);
```

### 3.7 Parent-Child-Hierarchie

```cpp
LPENTITY arm = nullptr, hand = nullptr;
Engine::CreateMesh(&arm, matArm);
Engine::CreateMesh(&hand, matHand);

Engine::SetEntityParent(hand, arm);       // hand folgt arm

Engine::PositionEntity(hand, 0.0f, 2.0f, 0.0f);  // relativ zu arm

float wx = Engine::EntityX(hand);         // Weltkoordinate abfragen
```

### 3.8 Render-to-Texture

```cpp
RenderTextureTarget* rtt = nullptr;
Engine::CreateRenderTexture(&rtt, 512, 512);
Engine::SetRTTClearColor(rtt, 0.1f, 0.1f, 0.2f, 1.0f);

LPENTITY rttCam = nullptr;
Engine::CreateCamera(&rttCam);
Engine::PositionEntity(rttCam, 0.0f, 5.0f, -10.0f);

// Im Frame: zuerst in RTT rendern
Engine::SetRenderTarget(rtt, rttCam);
Engine::RenderWorld();
Engine::ResetRenderTarget();

// RTT-Ergebnis als Textur verwenden
LPTEXTURE rttTex = Engine::GetRTTTexture(rtt);
Engine::MaterialTexture(monitorMat, rttTex, 0);
```

### 3.9 Frame-Timing

```cpp
Core::BeginFrame();
float dt = (float)Timer::GetDeltaTime();

Engine::TurnEntity(mesh, 0.0f, 45.0f * dt, 0.0f);  // 45°/s

double fps = Timer::GetFPS();
Core::EndFrame();
```

---

*OYNAME-3DEngine · Anwenderhandbuch · DirectX 11 / C++ / HLSL*
