// ortho_rtt_showcase.cpp
//
// Zeigt den Unterschied zwischen perspektivischer und orthografischer Darstellung:
//
// Beide Kameras stehen an exakt derselben Position mit derselben Ausrichtung.
// Der Unterschied ist ausschliesslich die Projektion:
//   Hauptkamera    = Perspektiv  -> Tiefeneindruck, Fluchtpunkte
//   OrthoCam (RTT) = Orthografisch -> parallele Linien, kein Tiefeneindruck
//
// Die RTT-Textur landet auf einem grossen Wuerfel rechts im Bild.
// Die OrthoCam rendert den RTT-Wuerfel selbst NICHT (wuerde Endlosschleife erzeugen).

#include "gidx.h"
#include "geometry.h"
#include <cmath>

void main(LPVOID hwnd)
{
    Debug::Log("ortho_rtt_showcase.cpp: main() gestartet");

    Engine::Graphics(1280, 720, true);

    // ---- Hauptkamera --------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 10.0f, -20.0f);
    Engine::LookAt(camera, 0.0f, 0.0f, 0.0f);

    // ---- Licht --------------------------------------------------------
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::LightColor(light, 1.0f, 0.95f, 0.85f);
    Engine::PositionEntity(light, 8.0f, 14.0f, -6.0f);
    Engine::LookAt(light, 0.0f, 0.0f, 0.0f);
    Engine::SetDirectionalLight(light);
    Engine::LightShadowOrthoSize(light, 24.0f);
    Engine::LightShadowPlanes(light, 1.0f, 50.0f);
    Engine::SetAmbientColor(0.42f, 0.42f, 0.48f);

    // ---- RTT ----------------------------------------------------------
    LPRENDERTARGET rtt = nullptr;
    Engine::CreateRenderTexture(&rtt, 512, 512);
    Engine::SetRTTClearColor(rtt, 0.04f, 0.04f, 0.10f, 1.0f);

    // ---- Orthografische RTT-Kamera ------------------------------------
    // Exakt dieselbe Position und LookAt wie die Hauptkamera.
    // Nur die Projektion ist anders: orthografisch statt perspektivisch.
    // LAYER_DEFAULT: rendert die Szene, aber NICHT den RTT-Wuerfel (der ist LAYER_FX).
    const float SCENE_SIZE = 14.0f;

    LPENTITY orthoCam = nullptr;
    Engine::CreateCamera(&orthoCam);
    Engine::PositionEntity(orthoCam, 0.0f, 10.0f, -20.0f);
    Engine::LookAt(orthoCam, 0.0f, 0.0f, 0.0f);
    Engine::SetCameraOrtho(orthoCam, SCENE_SIZE, SCENE_SIZE, 0.5f, 60.0f);
    Engine::CameraCullMask(orthoCam, LAYER_DEFAULT);
    Engine::SetCamera(camera);   // CreateCamera setzt intern SetCamera -- zuruecksetzen

    // ---- Boden --------------------------------------------------------
    LPMATERIAL groundMat = nullptr;
    Engine::CreateMaterial(&groundMat);
    Engine::MaterialColor(groundMat, 0.18f, 0.18f, 0.24f, 1.0f);
    Engine::MaterialUsePBR(groundMat, true);
    Engine::MaterialMetallic(groundMat, 0.0f);
    Engine::MaterialRoughness(groundMat, 0.85f);
    Engine::MaterialReceiveShadows(groundMat, true);

    LPENTITY ground = nullptr;
    CreateCube(&ground, groundMat);
    Engine::EntityCastShadows(ground, false);
    Engine::PositionEntity(ground, 0.0f, -0.6f, 0.0f);
    Engine::ScaleEntity(ground, 14.0f, 0.3f, 14.0f);
    // LAYER_DEFAULT: beide Kameras sehen den Boden

    // ---- Sechs bunte rotierende Wuerfel -------------------------------
    struct SceneCube
    {
        LPENTITY mesh;
        float    orbitR;
        float    speed;
        float    angle;
        float    bobPhase;
    };

    const int   NUM_CUBES = 6;
    SceneCube   sceneCubes[NUM_CUBES];

    const float cubeColors[NUM_CUBES][3] = {
        { 1.0f, 0.20f, 0.20f },   // Rot
        { 0.2f, 0.85f, 0.20f },   // Gruen
        { 0.2f, 0.40f, 1.00f },   // Blau
        { 1.0f, 0.80f, 0.10f },   // Gelb
        { 0.9f, 0.20f, 0.90f },   // Magenta
        { 0.1f, 0.90f, 0.90f },   // Cyan
    };

    for (int i = 0; i < NUM_CUBES; ++i)
    {
        LPMATERIAL mat = nullptr;
        Engine::CreateMaterial(&mat);
        Engine::MaterialColor(mat,
            cubeColors[i][0], cubeColors[i][1], cubeColors[i][2], 1.0f);
        Engine::MaterialUsePBR(mat, true);
        Engine::MaterialMetallic(mat, 0.1f);
        Engine::MaterialRoughness(mat, 0.4f);
        Engine::MaterialReceiveShadows(mat, true);
        Engine::MaterialEmissiveColor(mat,
            cubeColors[i][0] * 0.25f,
            cubeColors[i][1] * 0.25f,
            cubeColors[i][2] * 0.25f, 0.5f);

        LPENTITY mesh = nullptr;
        CreateCube(&mesh, mat);
        Engine::EntityCastShadows(mesh, true);
        Engine::ScaleEntity(mesh, 0.8f, 0.8f, 0.8f);
        // LAYER_DEFAULT: von beiden Kameras gerendert

        sceneCubes[i].mesh = mesh;
        sceneCubes[i].orbitR = 2.0f + static_cast<float>(i % 3) * 0.9f;
        sceneCubes[i].speed = 0.5f + static_cast<float>(i) * 0.11f;
        sceneCubes[i].angle = i * DirectX::XM_2PI / static_cast<float>(NUM_CUBES);
        sceneCubes[i].bobPhase = i * DirectX::XM_2PI / static_cast<float>(NUM_CUBES);
    }

    // ---- Grosser RTT-Wuerfel ------------------------------------------
    // LAYER_FX: nur die Hauptkamera sieht ihn.
    // Die OrthoCam rendert ihn nicht -- verhindert Endlosschleife.
    LPMATERIAL rttMat = nullptr;
    Engine::CreateMaterial(&rttMat);
    Engine::MaterialColor(rttMat, 1.0f, 1.0f, 1.0f, 1.0f);
    Engine::MaterialUsePBR(rttMat, true);
    Engine::MaterialMetallic(rttMat, 0.05f);
    Engine::MaterialRoughness(rttMat, 0.25f);
    Engine::MaterialReceiveShadows(rttMat, false);
    Engine::MaterialTexture(rttMat, Engine::GetRTTTexture(rtt));

    LPENTITY rttCube = nullptr;
    CreateCube(&rttCube, rttMat);
    Engine::EntityCastShadows(rttCube, false);
    Engine::PositionEntity(rttCube, 10.0f, 5.0f, 2.0f);
    Engine::ScaleEntity(rttCube, 5.0f, 5.0f, 5.0f);
    Engine::EntityLayer(rttCube, LAYER_FX);

    // Hauptkamera sieht LAYER_DEFAULT + LAYER_FX
    Engine::CameraCullMask(camera, LAYER_DEFAULT | LAYER_FX);

    Debug::Log("ortho_rtt_showcase.cpp: Szene aufgebaut");

    // ---- Hauptschleife ------------------------------------------------
    float timeAcc = 0.0f;

    while (Windows::MainLoop())
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Timer::GetDeltaTime());
        timeAcc += dt;

        // Bunte Wuerfel animieren
        for (int i = 0; i < NUM_CUBES; ++i)
        {
            sceneCubes[i].angle += sceneCubes[i].speed * dt;
            const float ox = sceneCubes[i].orbitR * std::sin(sceneCubes[i].angle);
            const float oz = sceneCubes[i].orbitR * std::cos(sceneCubes[i].angle);
            const float oy = 0.5f + 0.35f * std::sin(timeAcc * 1.3f + sceneCubes[i].bobPhase);
            Engine::PositionEntity(sceneCubes[i].mesh, ox, oy, oz);
            Engine::TurnEntity(sceneCubes[i].mesh, 28.0f * dt, 42.0f * dt, 0.0f);
        }

        // RTT-Wuerfel langsam drehen
        //Engine::TurnEntity(rttCube, 0.0f, 9.0f * dt, 0.0f);

        // Pass 1: OrthoCam (gleiche Position wie Hauptkamera) rendert Szene in RTT
        // Rendert nur LAYER_DEFAULT -- der RTT-Wuerfel (LAYER_FX) bleibt aussen vor
        Engine::SetCamera(orthoCam);
        Engine::SetRenderTarget(rtt, orthoCam);
        Engine::UpdateWorld();
        Engine::RenderWorld();

        // Pass 2: Hauptkamera rendert alles inkl. RTT-Wuerfel
        Engine::ResetRenderTarget();
        Engine::SetCamera(camera);
        Engine::Cls(8, 8, 16);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    Engine::ReleaseRenderTexture(&rtt);
    Debug::Log("ortho_rtt_showcase.cpp: main() beendet");
}
