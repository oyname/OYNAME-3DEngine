// transparent_example.cpp
//
// Beispiel: Transparente Objekte mit der OYNAME-Engine
//
// Szene:
//   - 1 opaker Boden (Plate)
//   - 1 opaker roter Wuerfel dahinter  (zeigt durch die transparenten Objekte hindurch)
//   - 3 transparente Wuerfel in verschiedenen Tiefen und Farben
//     (Blau weit, Gruen mittig, Gelb nah)
//
// Die Engine sortiert transparente Objekte automatisch back-to-front.
// Kein manuelles Eingreifen noetig.
//
// Relevante API-Aufrufe fuer Transparenz:
//   MaterialTransparent(mat, true)         -- Objekt in den Transparent-Pass
//   MaterialColor(mat, r, g, b, alpha)     -- alpha < 1.0 = halbdurchsichtig
//
// Dateien die du brauchst (sind alle im Projekt):
//   geometry.h / geometry.cpp              -- CreateCube(), CreatePlate()
//   gidx.h                                 -- gesamte Engine-API

#include "gidx.h"
#include "geometry.h"


// -----------------------------------------------------------------------
// Hilfsfunktion: opaken Wuerfel erstellen
// -----------------------------------------------------------------------
static void MakeOpaqueCube(LPENTITY* mesh, float r, float g, float b)
{
    LPMATERIAL mat = nullptr;
    Engine::CreateMaterial(&mat);
    Engine::MaterialColor(mat, r, g, b, 1.0f);   // alpha = 1.0 -> opak
    Engine::MaterialUsePBR(mat, false);

    CreateCube(mesh, mat);
}

// -----------------------------------------------------------------------
// Hilfsfunktion: transparenten Wuerfel erstellen
//
// alpha: 0.0 = unsichtbar, 1.0 = voll opak. Typisch 0.3 - 0.7 fuer Glas.
// -----------------------------------------------------------------------
static void MakeTransparentCube(LPENTITY* mesh, float r, float g, float b, float alpha)
{
    LPMATERIAL mat = nullptr;
    Engine::CreateMaterial(&mat);

    // Farbe UND Alpha setzen -- alpha < 1.0 macht das Objekt halbtransparent
    Engine::MaterialColor(mat, r, g, b, alpha);

    // MaterialTransparent(true) verschiebt das Objekt in den Transparent-Pass.
    // Der RenderManager sortiert diesen Pass automatisch back-to-front
    // (nach Distanz Kamera -> Mesh-Zentrum).
    Engine::MaterialTransparent(mat, true);

    Engine::MaterialUsePBR(mat, false);

    CreateCube(mesh, mat);
}

// -----------------------------------------------------------------------
// Hauptfunktion (wird von WinMain auf einem separaten Thread gestartet)
// -----------------------------------------------------------------------
void main()
{
    Debug::Log("transparent_example.cpp: main() gestartet");

    // ---- Grafikmodus ------------------------------------------------
    Engine::Graphics(1280, 720, true);

    // ---- Kamera -----------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera,  0.0f, 4.0f, -14.0f);
    Engine::LookAt(camera,          0.0f, 0.0f,   0.0f);

    // ---- Licht ------------------------------------------------------
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(light, 5.0f, 10.0f, -5.0f);
    Engine::LookAt(light, 0.0f, 0.0f, 0.0f);
    Engine::LightColor(light, 1.0f, 0.95f, 0.85f);
    Engine::SetDirectionalLight(light);
    Engine::SetAmbientColor(0.25f, 0.25f, 0.25f);

    // ---- Boden (opak) -----------------------------------------------
    LPENTITY floor = nullptr;
    Engine::CreateMesh(&floor);
    {
        LPMATERIAL matFloor = nullptr;
        Engine::CreateMaterial(&matFloor);
        Engine::MaterialColor(matFloor, 0.4f, 0.4f, 0.4f, 1.0f);
        Engine::MaterialUsePBR(matFloor, false);
        CreatePlate(&floor);
        Engine::SetSlotMaterial(floor, 0, matFloor);
    }
    Engine::ScaleEntity(floor, 8.0f, 1.0f, 8.0f);
    Engine::PositionEntity(floor, 0.0f, -2.0f, 0.0f);

    // ---- Opaker Wuerfel (Referenz) ----------------------------------
    // Dieser Wuerfel ist vollstaendig opak und steht hinter den
    // transparenten Wuerfeln. Man kann ihn durch sie hindurchsehen.
    LPENTITY solidCube = nullptr;
    MakeOpaqueCube(&solidCube, 0.85f, 0.15f, 0.15f);   // rot, opak
    Engine::PositionEntity(solidCube, 0.0f, 0.0f, 5.0f);
    Engine::ScaleEntity(solidCube, 1.2f, 1.2f, 1.2f);

    // ---- Transparente Wuerfel ---------------------------------------
    // Drei Wuerfel auf der Z-Achse gestaffelt.
    // Der RenderManager berechnet pro Frame die Distanz zur Kamera und
    // zeichnet sie in der Reihenfolge weit -> nah (back-to-front).
    // Das ist notwendig damit Alpha-Blending korrekt akkumuliert.

    LPENTITY cubeFar  = nullptr;    // blau,  weit  (Z =  3)
    LPENTITY cubeMid  = nullptr;    // gruen, mitte (Z =  0)
    LPENTITY cubeNear = nullptr;    // gelb,  nah   (Z = -3)

    MakeTransparentCube(&cubeFar,   0.2f, 0.4f, 1.0f, 0.45f);  // blau,  45% deckend
    MakeTransparentCube(&cubeMid,   0.2f, 0.9f, 0.3f, 0.50f);  // gruen, 50% deckend
    MakeTransparentCube(&cubeNear,  1.0f, 0.85f, 0.1f, 0.55f); // gelb,  55% deckend

    Engine::PositionEntity(cubeFar,   0.0f, 0.0f,  3.0f);
    Engine::PositionEntity(cubeMid,   0.0f, 0.0f,  0.0f);
    Engine::PositionEntity(cubeNear,  0.0f, 0.0f, -3.0f);

    Debug::Log("transparent_example.cpp: Szene aufgebaut - 1 opaker Referenzwuerfel, 3 transparente Wuerfel");

    // ---- Hauptschleife ----------------------------------------------
    float rotY = 0.0f;

    while (Windows::MainLoop())
    {
        Core::BeginFrame();

        float dt = static_cast<float>(Timer::GetDeltaTime());

        // Langsame Rotation aller drei transparenten Wuerfel
        rotY += 0.05f * dt;
        if (rotY > 360.0f) rotY -= 360.0f;

        Engine::RotateEntity(cubeFar,   0.0f, rotY,        0.0f);
        Engine::RotateEntity(cubeMid,   0.0f, rotY * 0.7f, 0.0f);
        Engine::RotateEntity(cubeNear,  0.0f, rotY * 1.3f, 0.0f);

        Engine::Cls(30, 30, 40);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    Debug::Log("transparent_example.cpp: main() beendet");
}
