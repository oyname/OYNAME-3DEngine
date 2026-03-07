// parenting_showcase.cpp
//
// Demonstriert Parent-Child-Hierarchien mit Animation:
//
// Sonnensystem-Aufbau:
//   Sonne (grosser gelber Wuerfel, dreht sich)
//     -> Erde (mittlerer blauer Wuerfel, umkreist Sonne)
//          -> Mond (kleiner grauer Wuerfel, umkreist Erde)
//               -> Mondflagge (winziger Wuerfel, klebt am Mond)
//     -> Asteroidenring (8 kleine Wuerfel, Kinder der Sonne -- erben ihre Rotation)
//
// Alle Positionen sind lokal zum jeweiligen Parent.
// TurnEntity auf den Parent dreht automatisch alle Kinder mit.

#include "gidx.h"
#include "geometry.h"
#include <cmath>

void main(LPVOID hwnd)
{
    Debug::Log("parenting_showcase.cpp: main() gestartet");

    Engine::Graphics(1280, 720, true);

    // ---- Kamera --------------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 18.0f, -26.0f);
    Engine::LookAt(camera, 0.0f, 0.0f, 0.0f);

    // ---- Licht ---------------------------------------------------------
    LPENTITY sunLight = nullptr;
    Engine::CreateLight(&sunLight, D3DLIGHT_DIRECTIONAL);
    Engine::LightColor(sunLight, 1.0f, 0.95f, 0.80f);
    Engine::PositionEntity(sunLight, 8.0f, 16.0f, -8.0f);
    Engine::LookAt(sunLight, 0.0f, 0.0f, 0.0f);
    Engine::SetDirectionalLight(sunLight);
    Engine::LightShadowOrthoSize(sunLight, 30.0f);
    Engine::LightShadowPlanes(sunLight, 1.0f, 60.0f);
    Engine::SetAmbientColor(0.12f, 0.12f, 0.18f);

    // ---- Boden ---------------------------------------------------------
    LPMATERIAL groundMat = nullptr;
    Engine::CreateMaterial(&groundMat);
    Engine::MaterialColor(groundMat, 0.12f, 0.12f, 0.18f, 1.0f);
    Engine::MaterialUsePBR(groundMat, true);
    Engine::MaterialMetallic(groundMat, 0.0f);
    Engine::MaterialRoughness(groundMat, 0.9f);
    Engine::MaterialReceiveShadows(groundMat, true);

    LPENTITY ground = nullptr;
    CreateCube(&ground, groundMat);
    Engine::EntityCastShadows(ground, false);
    Engine::PositionEntity(ground, 0.0f, -1.5f, 0.0f);
    Engine::ScaleEntity(ground, 40.0f, 0.4f, 40.0f);

    // ---- Sonne ---------------------------------------------------------
    LPMATERIAL sunMat = nullptr;
    Engine::CreateMaterial(&sunMat);
    Engine::MaterialColor(sunMat, 1.0f, 0.85f, 0.1f, 1.0f);
    Engine::MaterialUsePBR(sunMat, true);
    Engine::MaterialMetallic(sunMat, 0.0f);
    Engine::MaterialRoughness(sunMat, 0.5f);
    Engine::MaterialReceiveShadows(sunMat, false);
    Engine::MaterialEmissiveColor(sunMat, 1.0f, 0.7f, 0.0f, 1.2f);

    LPENTITY sun = nullptr;
    CreateSphere(&sun, sunMat);
    Engine::PositionEntity(sun, 0.0f, 5.5f, 0.0f);
    Engine::ScaleEntity(sun, 2.5f, 2.5f, 2.5f);

    // ---- Erde ----------------------------------------------------------
    // Position ist lokal zur Sonne: X=8 bedeutet 8 Einheiten von der Sonne entfernt.
    // Wenn die Sonne dreht, erbt die Erde diese Drehung automatisch.
    LPMATERIAL earthMat = nullptr;
    Engine::CreateMaterial(&earthMat);
    Engine::MaterialColor(earthMat, 0.15f, 0.40f, 0.85f, 1.0f);
    Engine::MaterialUsePBR(earthMat, true);
    Engine::MaterialMetallic(earthMat, 0.0f);
    Engine::MaterialRoughness(earthMat, 0.6f);
    Engine::MaterialReceiveShadows(earthMat, true);

    LPENTITY earth = nullptr;
    CreateSphere(&earth, earthMat);
    Engine::ScaleEntity(earth, 1.2f, 1.2f, 1.2f);
    Engine::PositionEntity(earth, 8.0f, 0.0f, 0.0f);  // lokal zur Sonne
    Engine::SetEntityParent(earth, sun);

    // ---- Mond ----------------------------------------------------------
    // Lokal zur Erde. Umkreist die Erde unabhaengig von der Sonnen-Rotation.
    LPMATERIAL moonMat = nullptr;
    Engine::CreateMaterial(&moonMat);
    Engine::MaterialColor(moonMat, 0.72f, 0.72f, 0.72f, 1.0f);
    Engine::MaterialUsePBR(moonMat, true);
    Engine::MaterialMetallic(moonMat, 0.05f);
    Engine::MaterialRoughness(moonMat, 0.8f);
    Engine::MaterialReceiveShadows(moonMat, true);

    LPENTITY moon = nullptr;
    CreateSphere(&moon, moonMat);
    Engine::ScaleEntity(moon, 0.5f, 0.5f, 0.5f);
    Engine::PositionEntity(moon, 2.8f, 0.0f, 0.0f);   // lokal zur Erde
    Engine::SetEntityParent(moon, earth);

    // ---- Flagge am Mond ------------------------------------------------
    // Kleiner Wuerfel der starr am Mond klebt -- keinerlei eigene Animation.
    LPMATERIAL flagMat = nullptr;
    Engine::CreateMaterial(&flagMat);
    Engine::MaterialColor(flagMat, 1.0f, 1.0f, 1.0f, 1.0f);
    Engine::MaterialUsePBR(flagMat, true);
    Engine::MaterialMetallic(flagMat, 0.0f);
    Engine::MaterialRoughness(flagMat, 0.3f);
    Engine::MaterialEmissiveColor(flagMat, 1.0f, 1.0f, 1.0f, 0.8f);

    LPENTITY flag = nullptr;
    CreateSphere(&flag, flagMat);
    Engine::ScaleEntity(flag, 0.18f, 0.18f, 0.18f);
    Engine::PositionEntity(flag, 0.0f, 0.5f, 0.0f);   // sitzt auf der Mondober flaeche
    Engine::SetEntityParent(flag, moon);

    // ---- Asteroidenring ------------------------------------------------
    // 8 Asteroiden als Kinder der Sonne -- erben deren Y-Rotation.
    // Eigene leichte Selbstrotation.
    const int NUM_ASTEROIDS = 8;
    LPENTITY asteroids[NUM_ASTEROIDS];

    LPMATERIAL asteroidMat = nullptr;
    Engine::CreateMaterial(&asteroidMat);
    Engine::MaterialColor(asteroidMat, 0.55f, 0.42f, 0.30f, 1.0f);
    Engine::MaterialUsePBR(asteroidMat, true);
    Engine::MaterialMetallic(asteroidMat, 0.1f);
    Engine::MaterialRoughness(asteroidMat, 0.85f);
    Engine::MaterialReceiveShadows(asteroidMat, true);

    for (int i = 0; i < NUM_ASTEROIDS; ++i)
    {
        const float a = DirectX::XM_2PI * static_cast<float>(i) / static_cast<float>(NUM_ASTEROIDS);
        const float r = 5.0f;   // Orbitradius um Sonne (lokal)

        LPENTITY ast = nullptr;
        CreateSphere(&ast, asteroidMat);
        const float s = 0.22f + 0.10f * std::sin(static_cast<float>(i) * 1.3f);
        Engine::ScaleEntity(ast, s, s, s);
        Engine::PositionEntity(ast,
            r * std::cos(a),
            0.3f * std::sin(static_cast<float>(i) * 0.9f),
            r * std::sin(a));
        Engine::SetEntityParent(ast, sun);
        asteroids[i] = ast;
    }

    Debug::Log("parenting_showcase.cpp: Hierachie aufgebaut");

    // ---- Hauptschleife ------------------------------------------------
    float timeAcc  = 0.0f;

    // Separate Winkel fuer Mond um Erde -- Earth-local, unabhaengig von Sonne
    float moonAngle = 0.0f;

    while (Windows::MainLoop())
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Timer::GetDeltaTime());
        timeAcc   += dt;
        moonAngle += 80.0f * dt;   // Grad pro Sekunde

        // Sonne dreht sich langsam um Y -- zieht alle Kinder mit
        Engine::TurnEntity(sun, 0.0f, 15.0f * dt, 0.0f);

        // Erde dreht sich eigenstaendig um sich selbst (local)
        Engine::TurnEntity(earth, 0.0f, 40.0f * dt, 0.0f);

        // Mond umkreist die Erde -- lokale Position zur Erde verschieben
        // Da earth sich bereits dreht, wird der Mond automatisch mitgezogen.
        // Wir wollen aber eine eigene Umlaufbahn -> Mond-Position wird manuell
        // pro Frame in Erd-lokalem Raum gesetzt.
        const float mRad = moonAngle * DirectX::XM_PI / 180.0f;
        Engine::PositionEntity(moon,
            2.8f * std::cos(mRad),
            0.0f,
            2.8f * std::sin(mRad));

        // Asteroiden rotieren leicht um sich selbst
        for (int i = 0; i < NUM_ASTEROIDS; ++i)
            Engine::TurnEntity(asteroids[i],
                20.0f * dt, 30.0f * dt * (1.0f + 0.3f * static_cast<float>(i)), 10.0f * dt);

        Engine::Cls(4, 4, 10);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    Debug::Log("parenting_showcase.cpp: main() beendet");
}
