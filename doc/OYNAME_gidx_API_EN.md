
OYNAME-3DEngine
gidx.h — API reference and application creation
Complete function reference · Step‑by‑step guide · Code examples

Version: 2025  ·  Namespace Engine  ·  #include "gidx.h"

# 1. Introduction
Die File gidx.h ist die einzige öffentliche Schnittstelle der OYNAME-3DEngine. Sie enthält ausschließlich inline-Functionen im Namespace Engine und verbirgt die gesamte DirectX-11-Implementierung hinter einfachen, direkten Aufrufen. Das Design folgt dem Vorbild von BlitzBasic: Eine Anwendung ruft Functionen auf und bekommt sofort das gewünschte Ergebnis, ohne COM-Interfaces, Ressourcen-Descriptionen oder Render-Zustände zu verwalten.

Um eine Anwendung zu schreiben, genügt ein einziger Include. Alle Typen, alle Functionen und alle Konstanten sind danach verfügbar.


#include "gidx.h"

// Alle Engine-Functionen sind im Namespace Engine:
using namespace Engine;  // optional fuer kuerzerencode

# 2. Creating an application — step by step
## 2.1 Aufbau einer main()-Function
Die Engine trennt Initialization und Game loop klar. WinMain in main.cpp übernimmt das Fenster-Management und die Engine-Erstellung automatisch. Die eigene main()-Function enthält nur noch Engine::Graphics(), die Szenen-Erstellung und den Frame-Loop. Ein minimales Gerüst sieht so aus:


#include "gidx.h"

int main()
{
// 1. Set graphics mode
Engine::Graphics(1280, 720);           // Fenstermodus
// Engine::Graphics(1920, 1080, false); // Vollbild

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
Engine::LoadTexturee(&tex, L"..\\media\\albedo.png");

// 5. Create material
LPMATERIAL mat = nullptr;
Engine::CreateMaterial(&mat);
Engine::MaterialSetAlbedo(mat, tex);

// 6. Create mesh (eigene Geometrie-Function)
LPENTITY mesh = nullptr;
Engine::CreateMesh(&mesh);
LPSURFACE surf = nullptr;
Engine::CreateSurface(&surf, mesh);
// ... Vertices und Dreiecke hinzufuegen ...
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


## 2.2 Frame loop in detail
Jeder Frame folgt einer festgelegten Reihenfolge. Core::BeginFrame() muss als Erstes aufgerufen werden, damit Timer-Werte gültig sind. Dann kommt die eigene Logik (Bewegung, KI, Input). Engine::Cls() löscht den Backbuffer. Engine::UpdateWorld() berechnet alle Entity-Transformationen und lädt Constant-Buffer. Engine::RenderWorld() führt Shadow Pass und Normal Pass aus. Engine::Flip() präsentiert den Backbuffer auf dem Monitor. Core::EndFrame() schließt die Frame-Zeitmessung ab.

# 3. Graphics initialization
## 3.1 Adapter and output
Wenn mehrere Grafikkarten oder Monitore im System vorhanden sind, kann vor Graphics() der gewünschte Adapter und die Ausgabe gewählt werden. CountGfxDrivers() gibt die Anzahl verfügbarer Adapter zurück. GfxDriverName() gibt den Namen des aktuellen Adapters aus. SetGfxDriver(index) wählt einen Adapter. SetOutput(index) wählt die Ausgabe für Vollbild.


# 4. Entity management — position, rotation, scale
Alle Functionen in diesem Abschnitt arbeiten mit jedem Entity-Typ: Meshes, Cameras und Lighter. LPENTITY ist ein Zeiger auf Entity. Alle Transformations-Functionen sind frame-rate-unabhängig, wenn sie mit Timer::GetDeltaTime() skaliert werden.

## 4.1 Positioning

## 4.2 Rotation and orientation

## 4.3 Hierarchy

## 4.4 Visibility and layers

# 5. Camera

Note:  Die Camera-Position und -Rotation werden über die allgemeinen Entity-Functionen PositionEntity und TurnEntity gesteuert. CreateCamera sets die Camera automatisch als aktive Camera.

# 6. Light

# 7. Mesh and geometry
## 7.1 Create mesh

## 7.2 Add vertex data
Alle AddVertex/VertexNormal/VertexColor/VertexTexCoord-Aufrufe müssen vor FillBuffer() erfolgen. Nach FillBuffer() sind die CPU-seitigen Arrays weiterhin vorhanden, können aber nur über UpdateVertexBuffer() oder UpdateColorBuffer() an die GPU übertragen werden.


## 7.3 Dynamic buffer updates

## 7.4 Asset sharing
ShareMeshAsset(source, target) lässt zwei Meshes dieselbe Geometrie teilen. Jede Instanz hat ihren eigenen Transform und eigene Materialien. Die Geometrie liegt nur einmal im GPU-Speicher.


LPENTITY original = nullptr, kopie = nullptr;
Engine::CreateMesh(&original);
// ... Geometrie fuer original aufbauen ...

Engine::CreateMesh(&kopie);
Engine::ShareMeshAsset(original, kopie);   // kopie nutzt Geometrie von original

// Jetzt unabhaengig positionieren:
Engine::PositionEntity(original, -3.0f, 0.0f, 5.0f);
Engine::PositionEntity(kopie,     3.0f, 0.0f, 5.0f);

// Vor DeleteMesh: Asset-Zeiger sichern, wenn andere Instanz weiterlebt:
// kopie->AsMesh()->meshRenderer.asset = nullptr;
// Engine::DeleteMesh(&kopie);


# 8. Material
## 8.1 Create material und zuweisen

## 8.2 Textureen

## 8.3 Farbe und klassische Eigenschaften

## 8.4 PBR-Parameters
PBR (Physically Based Rendering) muss explizit per MaterialUsePBR(mat, true) aktiviert werden. Der Versionard-Shader verwendet dann Cook-Torrance GGX statt Blinn-Phong. PBR und Legacy-Modus können im selben Frame auf unterschiedlichen Materialien gleichzeitig verwendet werden.


## 8.5 Transparenz und Alpha

# 9. Shader
Der Versionard-Shader (VertexShader.hlsl + PixelShader.hlsl) wird automatisch von der Engine geladen. Eigene Shader können erstellt werden, wenn besondere Vertex-Formate oder Shading-Effekte benötigt werden.



LPSHADER myShader = nullptr;
DWORD flags = Engine::CreateVertexFlags(
true,   // Position (immer true)
true,   // Normal
false,  // Color
true,   // UV1
true,   // UV2
true    // Tangent (fuer Normal-Mapping)
);

Engine::CreateShader(&myShader,
L"shaders/VertexShader.hlsl", "VS",
L"shaders/PixelShader.hlsl",  "PS",
flags);

LPMATERIAL mat = nullptr;
Engine::CreateMaterial(&mat, myShader);


# 10. Textureen — Prozedural und Pixel-Zugriff

# 11. Render‑to‑Texture (RTT)
RTT erlaubt es, eine komplette Szene in eine Texture zu rendern, die dann als Albedo für ein anderes Object verwendet werden kann. Typische Anwendungen sind Spiegel, Sicherheitskameras, Minimap-Rendering oder Post-Processing-Effekte.



// Spiegel-Example:
LPRENDERTARGET mirrorRTT = nullptr;
Engine::CreateRenderTexturee(&mirrorRTT, 512, 512);
Engine::SetRTTClearColor(mirrorRTT, 0.1f, 0.1f, 0.15f);

// Material fuer die Spiegelflaeche:
LPMATERIAL mirrorMat = nullptr;
Engine::CreateMaterial(&mirrorMat);
Engine::MaterialSetAlbedo(mirrorMat, Engine::GetRTTTexturee(mirrorRTT));

// Im Frame-Loop:
Engine::SetRenderTarget(mirrorRTT);    // In RTT rendern
Engine::RenderWorld();
Engine::ResetRenderTarget();           // Zurueck zum Backbuffer
Engine::RenderWorld();                 // Normaler Frame


# 12. Skeletal animation

# 13. Collision

# 14. Debug output

Note:  Die Engine-Coding-Regel schreibt vor, dass alle Debug::Log()-Aufrufe mit dem Filenamen beginnen. Example: Debug::Log("game.cpp: Mesh erstellt");

# 15. Complete Example — PBR-Szene mit Shadow Mapping
Dieses Example zeigt einen vollständigen Aufbau mit zwei PBR-Materialien, Directional Light mit Shadow Mapping, Parent/Child-Hierarchy und frame-rate-unabhängiger Animation.


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

// --- Textureen ---
LPTEXTURE albedo = nullptr, normal = nullptr, orm = nullptr;
Engine::LoadTexturee(&albedo, L"..\\media\\albedo.png");
Engine::LoadTexturee(&normal, L"..\\media\\normal.png");
Engine::LoadTexturee(&orm,    L"..\\media\\orm.png");

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
Engine::PositionEntity(child, 3.0f, 0.0f, 0.0f);  // Relativ zum Parent

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


# 16. Types and constants
## 16.1 Pointer types

## 16.2 Important enums and constants
