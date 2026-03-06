// ════════════════════════════════════════════════════════════════════════════
// main_rtt.cpp – RTT Demo
//
// Linker Würfel: dreht sich, wird von einer RTT-Kamera gefilmt.
// Rechter Würfel: steht still, trägt die RTT-Textur — zeigt live den linken Würfel.
// ════════════════════════════════════════════════════════════════════════════

#include "gidx.h"
#include "geometry.h"

int main()
{
    Engine::Graphics(1200, 650);

    // ── Haupt-Textur (Face) ───────────────────────────────────────────────────
    LPTEXTURE texFace = nullptr;
    Engine::LoadTexture(&texFace, L"..\\media\\color1.png");

    // ── Material für den linken Würfel (face.bmp) ─────────────────────────────
    LPMATERIAL materialLeft = nullptr;
    Engine::CreateMaterial(&materialLeft);
    Engine::MaterialTexture(materialLeft, texFace);

    // ── RTT anlegen: 512×512 Render-Textur ───────────────────────────────────
    LPRENDERTARGET rtt = nullptr;
    Engine::CreateRenderTexture(&rtt, 512, 512);
    Engine::SetRTTClearColor(rtt, 0.6f, 0.6f, 0.8f); // dunkles Blau als RTT-Hintergrund

    // ── Material für den rechten Würfel: bekommt RTT-Textur ──────────────────
    LPMATERIAL materialRight = nullptr;
    Engine::CreateMaterial(&materialRight);
    Engine::MaterialTexture(materialRight, Engine::GetRTTTexture(rtt));

    // ── Haupt-Kamera ──────────────────────────────────────────────────────────
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 0.0f, -10.0f);

    // ── RTT-Kamera: seitliche Perspektive auf den linken Würfel ──────────────
    // Diese Kamera rendert nur den linken Würfel in die RTT.
    LPENTITY rttCamera = nullptr;
    Engine::CreateCamera(&rttCamera);
    Engine::PositionEntity(rttCamera, 4.0f, 0.0f, 10.0f);
    Engine::TurnEntity(rttCamera, 0.0f, 90.0f, 0.0f);

    // ── Lichter ───────────────────────────────────────────────────────────────
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(light, 0.0f, 20.0f, 10.0f);
    Engine::RotateEntity(light, 90.0f, 0.0f, 0.0f);
    Engine::LightColor(light, 0.4f, 0.4f, 0.5f);

    LPENTITY light2 = nullptr;
    Engine::CreateLight(&light2, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(light2, 0.0f, 10.0f, -10.0f);
    Engine::RotateEntity(light2, -45.0f, 0.0f, 0.0f);
    Engine::LightColor(light2, 0.5f, 0.5f, 0.5f);

    Engine::SetDirectionalLight(light);
    Engine::SetAmbientColor(0.9f, 0.9f, 0.9f);

    // ── Linker Würfel – dreht sich, eigenes Material ──────────────────────────
    LPENTITY cubeLeft = nullptr;
    CreateCube(&cubeLeft, materialLeft);
    Engine::PositionEntity(cubeLeft, -4.0f, 0.0f, 10.0f);
    Engine::ScaleEntity(cubeLeft, 2.0f, 2.0f, 2.0f);
    Engine::EntityTexture(cubeLeft, texFace);
    Engine::EntityMaterial(cubeLeft, materialLeft);


    // ── Rechter Würfel – steht still, zeigt RTT-Textur ───────────────────────
    LPENTITY cubeRight = nullptr;
    CreateCube(&cubeRight, nullptr);
    Engine::PositionEntity(cubeRight, 4.0f, 0.0f, 5.0f);
    Engine::ScaleEntity(cubeRight, 2.0f, 2.0f, 2.0f);
    Engine::TurnEntity(cubeRight, 45.0f, 45.0f, 0.0f);

    // Material an Surface slot[]=texture
    //Engine::SurfaceMaterial(Engine::GetSurface(cubeRight), materialRight);
     
    // Material ist Standardmaterial 
    Engine::EntityMaterial(cubeRight, materialRight);

    Engine::DebugPrintScene();

    // ── Haupt-Loop ────────────────────────────────────────────────────────────
    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Timer::GetDeltaTime());

        // Linken Würfel drehen
        Engine::TurnEntity(cubeLeft, 60.0f * dt, 80.0f * dt, 0.0f);

        // ══ Pass 1: RTT-Pass ════════════════════════════════════════════════
        // Rendert den linken Würfel aus Sicht der RTT-Kamera in die Textur.
        // Der rechte Würfel wird in dieser Kamera nicht benötigt — er ist
        // sichtbar, aber aus dem Blickwinkel der RTT-Kamera nicht im Bild.
        Engine::SetCamera(rttCamera);
        Engine::SetRenderTarget(rtt, rttCamera);
        Engine::UpdateWorld();
        Engine::RenderWorld();   // → rendert in rtt (512×512)

        // ══ Pass 2: Normaler Frame ══════════════════════════════════════════
        Engine::ResetRenderTarget();
        Engine::SetCamera(camera);
        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();   // → rendert in Backbuffer (1200×650)
        Engine::Flip();

        Core::EndFrame();
    }

    // ── Aufräumen ─────────────────────────────────────────────────────────────
    Engine::ReleaseRenderTexture(&rtt);

    return 0;
}