// shadow_showcase.cpp
//
// Shadow Showcase fuer die OYNAME-3DEngine
//
// Zeigt PCF Shadow Mapping in Aktion:
//   - Ein grosser Boden empfaengt Schatten
//   - Ein Turm aus gestapelten Boxen wirft Schatten
//   - Drei einzelne Wuerfel auf dem Boden werfen und empfangen Schatten
//   - Das direktionale Licht kreist langsam um die Szene
//     --> die Schatten wandern live mit, weich durch PCF-Filterung
//
// Relevante API-Aufrufe:
//   SetDirectionalLight(light)          -- aktiviert Shadow Pass fuer dieses Licht
//   EntityCastShadows(mesh, true/false) -- Mesh im Shadow Pass rendern
//   MaterialReceiveShadows(mat, true)   -- Shadow Map im Pixel Shader auswerten
//   PositionEntity + LookAt pro Frame   -- Licht bewegen -> Schatten wandern

// !!
// als Faustregel : LightShadowOrthoSize sollte ungefähr dem Durchmesser der sichtbaren Szene entsprechen, 
// und LightShadowPlanes sollte den Near / Far - Bereich so eng wie möglich um die Szene legen.Beides zusammen
// holt das Maximum aus der vorhandenen Shadow Map Auflösung heraus.

#include "gidx.h"
#include "geometry.h"

#include <cmath>

// -----------------------------------------------------------------------
// Hilfsfunktion: opaken Wuerfel mit Farbe erstellen
// -----------------------------------------------------------------------
static LPENTITY MakeBox(float r, float g, float b,
    float posX, float posY, float posZ,
    float scaleX, float scaleY, float scaleZ,
    bool castShadow = true, bool receiveShadow = true)
{
    LPMATERIAL mat = nullptr;
    Engine::CreateMaterial(&mat);
    Engine::MaterialColor(mat, r, g, b, 1.0f);
    Engine::MaterialUsePBR(mat, true);
    Engine::MaterialMetallic(mat, 0.0f);
    Engine::MaterialRoughness(mat, 0.75f);
    Engine::MaterialReceiveShadows(mat, receiveShadow);

    LPENTITY mesh = nullptr;
    CreateCube(&mesh, mat);

    Engine::EntityCastShadows(mesh, castShadow);
    Engine::PositionEntity(mesh, posX, posY, posZ);
    Engine::ScaleEntity(mesh, scaleX, scaleY, scaleZ);

    return mesh;
}

// -----------------------------------------------------------------------
// Hauptfunktion
// -----------------------------------------------------------------------
void main(LPVOID hwnd)
{
    Debug::Log("shadow_showcase.cpp: main() gestartet");

    Engine::Graphics(1280, 720, true);

    // ---- Kamera -----------------------------------------------------
    // Leicht von oben schauend, damit man Schatten gut sieht
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 12.0f, -22.0f);
    Engine::LookAt(camera, 0.0f, 0.0f, 0.0f);

    // ---- Licht ------------------------------------------------------
    // Direktionales Licht -- wird jeden Frame neu positioniert
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::LightColor(light, 1.0f, 0.93f, 0.78f);  // warmes Sonnenlicht
    Engine::SetDirectionalLight(light);
    Engine::LightShadowOrthoSize(light, 20.0f);      // Szene ist ~12x12 Einheiten -> 20 gibt etwas Rand
    Engine::LightShadowPlanes(light, 1.0f, 60.0f);   // Near/Far eng um die Szene -> bessere Tiefenpraezision
    Engine::SetAmbientColor(0.32f, 0.34f, 0.38f);   // kuehles dunkles Ambient

    // ---- Boden -------------------------------------------------------
    // Grosser Boden empfaengt Schatten, wirft aber keinen
    LPENTITY floor = nullptr;
    Engine::CreateMesh(&floor);
    {
        LPMATERIAL matFloor = nullptr;
        Engine::CreateMaterial(&matFloor);
        Engine::MaterialColor(matFloor, 0.55f, 0.52f, 0.48f, 1.0f);  // helles Beige
        Engine::MaterialUsePBR(matFloor, true);
        Engine::MaterialMetallic(matFloor, 0.0f);
        Engine::MaterialRoughness(matFloor, 0.95f);
        Engine::MaterialReceiveShadows(matFloor, true);

        CreatePlate(&floor);
        Engine::SurfaceMaterial(Engine::EntitySurface(floor, 0), matFloor);
    }
    Engine::EntityCastShadows(floor, false);   // Boden wirft keinen Schatten
    Engine::ScaleEntity(floor, 12.0f, 1.0f, 12.0f);
    Engine::PositionEntity(floor, 0.0f, -0.01f, 0.0f);

    // ---- Turm (gestapelte Boxen) ------------------------------------
    // 5 Boxen uebereinander, leicht versetzt fuer mehr Dramatik
    // Der Turm erzeugt lange, wandernde Schatten auf dem Boden
    const int   TOWER_LEVELS = 5;
    const float TOWER_BASE = 1.0f;    // halbe Breite/Tiefe
    LPENTITY    tower[TOWER_LEVELS];

    // Farben von dunkel unten nach hell oben
    const float towerColors[TOWER_LEVELS][3] = {
        { 0.25f, 0.22f, 0.20f },
        { 0.35f, 0.31f, 0.28f },
        { 0.45f, 0.40f, 0.36f },
        { 0.55f, 0.50f, 0.45f },
        { 0.65f, 0.60f, 0.55f }
    };

    for (int i = 0; i < TOWER_LEVELS; ++i)
    {
        float scale = TOWER_BASE - i * 0.1f;   // leicht verjuengt nach oben
        float posY = i * 2.0f + scale;        // jede Box sitzt auf der naechsten

        tower[i] = MakeBox(
            towerColors[i][0], towerColors[i][1], towerColors[i][2],
            -4.0f, posY, 2.0f,                 // leicht links und vorne
            scale, scale, scale,
            true, true
        );
    }

    // ---- Drei einzelne Wuerfel auf dem Boden ------------------------
    // Unterschiedliche Groessen und Farben fuer abwechslungsreiche Schatten
    LPENTITY cubeA = MakeBox(0.80f, 0.25f, 0.20f,  // rot
        2.0f, 0.6f, 1.0f,
        0.6f, 0.6f, 0.6f);

    LPENTITY cubeB = MakeBox(0.20f, 0.55f, 0.80f,  // blau
        4.5f, 1.0f, -1.5f,
        1.0f, 1.0f, 1.0f);

    LPENTITY cubeC = MakeBox(0.30f, 0.75f, 0.40f,  // gruen
        1.0f, 0.35f, -3.0f,
        0.35f, 0.35f, 0.35f);

    // ---- Kleiner Wuerfel der KEINEN Schatten wirft ------------------
    // Demonstriert EntityCastShadows(false) -- er empfaengt Schatten,
    // aber sein eigener Schatten fehlt auf dem Boden
    LPENTITY cubeNoShadow = MakeBox(0.90f, 0.85f, 0.10f,  // gelb
        -1.5f, 0.4f, -3.5f,
        0.4f, 0.4f, 0.4f,
        false,   // castShadow = false
        true);   // receiveShadow = true

    Debug::Log("shadow_showcase.cpp: Szene aufgebaut");
    Debug::Log("shadow_showcase.cpp: Gelber Wuerfel wirft KEINEN Schatten (EntityCastShadows=false)");

    // ---- Licht-Orbit-Parameter --------------------------------------
    float lightAngle = 45.0f;    // Startwinkel in Grad
    float lightSpeed = 18.0f;    // Grad pro Sekunde
    float lightRadius = 16.0f;    // horizontaler Abstand vom Zentrum
    float lightHeight = 14.0f;    // Hoehe des Lichts

    // ---- Hauptschleife ----------------------------------------------
    while (Windows::MainLoop())
    {
        Core::BeginFrame();

        float dt = static_cast<float>(Timer::GetDeltaTime());

        // Lichtposition jeden Frame neu berechnen und setzen
        // Light::Update() liest automatisch Position + LookAt aus dem Transform
        lightAngle += lightSpeed * dt;
        if (lightAngle >= 360.0f) lightAngle -= 360.0f;

        float rad = lightAngle * DirectX::XM_PI / 180.0f;
        float lx = lightRadius * std::sin(rad);
        float lz = -(lightRadius * std::cos(rad));

        Engine::PositionEntity(light, lx, lightHeight, lz);
        Engine::LookAt(light, 0.0f, 0.0f, 0.0f);

        Engine::Cls(18, 20, 26);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    Debug::Log("shadow_showcase.cpp: main() beendet");
}
