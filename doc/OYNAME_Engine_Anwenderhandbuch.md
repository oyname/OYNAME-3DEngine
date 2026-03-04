# OYNAME-3DEngine --- Anwenderhandbuch

DirectX 11 · C++ · BlitzBasic-inspirierte API

------------------------------------------------------------------------

## 1. Einstieg

Die **OYNAME-3DEngine** stellt eine einfache, BlitzBasic-inspirierte API
für 3D-Anwendungen bereit.

Die komplette Engine wird über **eine einzige Header-Datei** verwendet:

``` cpp
#include "gidx.h"
```

Die Anwendung arbeitet ausschließlich mit Funktionen aus dem Namespace:

    Engine::

Hilfsfunktionen befinden sich zusätzlich in:

    Windows::
    Core::

Die Engine kapselt intern:

-   DirectX 11
-   GPU-Ressourcen
-   Shader
-   Render-Pipeline

Der Anwendungscode sieht **keine DirectX-Typen** und muss keine
GPU-Ressourcen selbst verwalten.

------------------------------------------------------------------------

## 1.1 Interne Architektur (Kurzüberblick)

Intern ist die Engine in drei Schichten aufgebaut:

    Application
        │
        ▼
    gidx API
        │
        ▼
    Engine Manager Layer
        │
        ▼
    DirectX‑11 Backend

### API-Schicht

Die Datei `gidx.h` stellt die gesamte öffentliche API bereit.

Sie übersetzt einfache Funktionsaufrufe wie:

    CreateMesh
    CreateMaterial
    RenderWorld

in interne Manager‑Operationen.

### Manager-Schicht

Die Engine-Logik wird von mehreren Managern organisiert:

  Manager              Aufgabe
  -------------------- ----------------------------------------------
  ObjectManager        Verwaltung aller Engine-Objekte
  RenderManager        Steuerung der Renderpipeline
  ShaderManager        Kompilierung und Verwaltung von HLSL-Shadern
  TexturePool          globaler Texture-Pool
  BufferManager        Erstellung von Vertex- und Constant-Buffers
  InputLayoutManager   Erstellung von Vertex-Layouts

### Backend-Schicht

Das Backend kapselt alle DirectX-Aufrufe.

Typische Backend-Klassen:

    GDXDevice
    Dx11RenderBackend
    SurfaceGpuBuffer
    Dx11EntityGpuData
    Dx11MaterialGpuData
    Dx11ShadowMap

------------------------------------------------------------------------

## 1.2 Minimales Programm

``` cpp
int main()
{
    Engine::Graphics(1280, 720);

    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 0.0f, -5.0f);

    while (Windows::MainLoop())
    {
        Engine::Cls(0, 64, 128);

        Engine::UpdateWorld();
        Engine::RenderWorld();

        Engine::Flip();
    }

    return 0;
}
```

------------------------------------------------------------------------

## 2. Engine-Start

  Funktion           Beschreibung
  ------------------ ------------------------------------------------
  `Graphics(w, h)`   Fenster öffnen und DirectX initialisieren
  `Cls(r,g,b)`       Backbuffer löschen
  `UpdateWorld()`    Transformationen und Kameradaten aktualisieren
  `RenderWorld()`    gesamte Renderpipeline ausführen
  `Flip()`           Bild anzeigen (Present)
  `SetVSync(bool)`   VSync aktivieren/deaktivieren

### Renderpipeline

`RenderWorld()` führt intern folgende Schritte aus:

    UpdateWorld
    RenderShadowPass
    BuildRenderQueue
    FlushOpaqueQueue
    FlushTransparentQueue
    Present

Der Anwendungscode muss diese Schritte **nicht selbst steuern**.

------------------------------------------------------------------------

## 3. Entity-Transformation

Transformationen funktionieren für **Mesh, Kamera und Licht identisch**.

  Funktion          Beschreibung
  ----------------- --------------------------
  PositionEntity    absolute Position setzen
  MoveEntity        relativ bewegen
  RotateEntity      absolute Rotation
  TurnEntity        relative Rotation
  ScaleEntity       Skalierung
  LookAt            auf Ziel ausrichten
  EntityX/Y/Z       Weltposition abfragen
  EntityLayer       Layer setzen
  SetEntityParent   Parent setzen
  DetachEntity      Parent entfernen
  ShowEntity        Sichtbarkeit

------------------------------------------------------------------------

## 4. Kamera

  Funktion         Beschreibung
  ---------------- --------------------------
  CreateCamera     Kamera erzeugen
  SetCamera        aktive Kamera setzen
  CameraCullMask   Layer-Culling definieren

------------------------------------------------------------------------

## 5. Licht

  Funktion              Beschreibung
  --------------------- ----------------------
  CreateLight           Licht erzeugen
  LightColor            Lichtfarbe
  SetDirectionalLight   Schattenlicht setzen
  SetAmbientColor       Ambient-Farbe
  EntityCastShadows     Mesh wirft Schatten

------------------------------------------------------------------------

## 6. Mesh & Surface

Ein Mesh besteht aus einer oder mehreren **Surfaces (Submeshes)**.

  Funktion          Beschreibung
  ----------------- ---------------------
  CreateMesh        neues Mesh
  CreateSurface     neue Surface
  AddVertex         Vertex hinzufügen
  VertexNormal      Normale setzen
  VertexColor       Vertexfarbe
  VertexTexCoord    UV Slot 0
  VertexTexCoord2   UV Slot 1
  AddTriangle       Dreieck definieren
  FillBuffer        GPU-Buffer erzeugen
  SurfaceMaterial   Material setzen

------------------------------------------------------------------------

## 7. Shader

Eigene Shader können geladen werden.

  Funktion            Beschreibung
  ------------------- ----------------------
  CreateShader        Shader kompilieren
  CreateVertexFlags   Vertexformat-Bitmask

Der Standardshader erwartet:

    Position
    Normal
    Tangent
    Color
    UV1
    UV2

------------------------------------------------------------------------

## 8. Material

Materialien beschreiben das Aussehen einer Surface.

### Textur-Slots

  Slot   Bedeutung
  ------ -----------------------------
  0      Albedo
  1      zweite Textur (Decal/Blend)
  2      Normal Map
  3      ORM Map
  4      Occlusion Map (optional)
  5      Roughness Map (optional)
  6      Metallic Map (optional)
  16     Shadow Map (intern)

Wenn eine **ORM-Map** gesetzt ist, werden die separaten Maps ignoriert.

### Materialfunktionen

  Funktion                 Beschreibung
  ------------------------ -------------------
  CreateMaterial           Material erzeugen
  MaterialTexture          Textur zuweisen
  MaterialSetAlbedo        Albedo
  MaterialSetNormal        Normal Map
  MaterialSetORM           ORM Map
  MaterialMetallic         Metallic
  MaterialRoughness        Roughness
  MaterialNormalScale      Normalstärke
  MaterialEmissiveColor    Emissive
  MaterialBlendMode        Blend-Modus
  MaterialBlendFactor      Blend-Faktor
  MaterialAlphaTest        Alpha-Test
  MaterialTransparent      Transparenz
  MaterialReceiveShadows   Shadow-Auswertung
  MaterialUVTilingOffset   UV-Transformation

------------------------------------------------------------------------

## 9. Texturen

  Funktion        Beschreibung
  --------------- -------------------
  LoadTexture     Datei laden
  CreateTexture   leere Textur
  LockBuffer      CPU-Zugriff
  UnlockBuffer    GPU aktualisieren
  SetPixel        Pixel setzen

------------------------------------------------------------------------

## 10. Render‑to‑Texture

  Funktion              Beschreibung
  --------------------- ------------------
  CreateRenderTexture   RTT erzeugen
  SetRenderTarget       RTT aktivieren
  ResetRenderTarget     Backbuffer
  GetRTTTexture         RTT als Textur
  SetRTTClearColor      Hintergrundfarbe

------------------------------------------------------------------------

## 11. Layer-System

Layer erlauben selektives Rendern.

  Funktion         Beschreibung
  ---------------- ---------------
  EntityLayer      Layer setzen
  CameraCullMask   Kamera-Filter

Beispiele:

-   HUD-Layer
-   Minimap-Kamera
-   spezielle RTT-Kameras

------------------------------------------------------------------------

## 12. Frame-Timing

``` cpp
Core::BeginFrame();

float dt = (float)Timer::GetDeltaTime();
Engine::TurnEntity(mesh, 0.0f, 45.0f * dt, 0.0f);

double fps = Timer::GetFPS();

Core::EndFrame();
```

------------------------------------------------------------------------

OYNAME‑3DEngine · Anwenderhandbuch · DirectX 11 / C++ / HLSL
