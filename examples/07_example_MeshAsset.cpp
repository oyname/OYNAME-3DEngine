#include "gidx.h"
#include "geometry.h"

int main()
{
    Engine::Graphics(1280, 720);

    // Kamera
    LPENTITY cam = nullptr;
    Engine::CreateCamera(&cam);
    Engine::PositionEntity(cam, 0.0f, 2.0f, -8.0f);
    Engine::LookAt(cam, 0.0f, 1.0f, 0.0f);
    Engine::SetCamera(cam);

    // Licht
    LPENTITY sun = nullptr;
    Engine::CreateLight(&sun, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(sun, 10.0f, 10.0f, -10.0f);
    Engine::LookAt(sun, 0.0f, 0.0f, 0.0f);
    Engine::LightColor(sun, 2.0f, 2.0f, 2.0f);
    Engine::SetDirectionalLight(sun);
    Engine::SetAmbientColor(0.2f, 0.2f, 0.2f);

    // Texturen
    LPTEXTURE texA = nullptr; Engine::LoadTexture(&texA, L"..\\media\\dx.bmp");
    LPTEXTURE texB = nullptr; Engine::LoadTexture(&texB, L"..\\media\\face.bmp");

    // Materialien (unterschiedliche BaseColor-Textur)
    LPMATERIAL matA = nullptr; Engine::CreateMaterial(&matA);
    Engine::MaterialRoughness(matA, 0.6f);
    Engine::MaterialMetallic(matA, 0.0f);
    Engine::MaterialSetAlbedo(matA, texA);

    LPMATERIAL matB = nullptr; Engine::CreateMaterial(&matB);
    Engine::MaterialRoughness(matB, 0.6f);
    Engine::MaterialMetallic(matB, 0.0f);
    Engine::MaterialSetAlbedo(matB, texB);

    // ------------------------------------------------------------------
    // 1) Entity A: Würfel erzeugen -> erzeugt Geometrie (MeshAsset + Slot 0)
    // ------------------------------------------------------------------
    LPENTITY cubeA = nullptr;
    CreateCube(&cubeA, matA);            // erzeugt Mesh + Surface(slot0) + Geometrie
    Engine::PositionEntity(cubeA, -1.5f, 1.0f, 0.0f);

    // ------------------------------------------------------------------
    // 2) Entity B: zweites Mesh erstellen, aber Asset von A teilen
    // ------------------------------------------------------------------
    LPENTITY cubeB = nullptr;
    Engine::CreateMesh(&cubeB, matB);    // Mesh-Instanz B
    Engine::PositionEntity(cubeB, 1.5f, 1.0f, 0.0f);

    // Asset teilen (beide zeigen auf denselben MeshAsset)
    Mesh* mA = cubeA->AsMesh();
    Mesh* mB = cubeB->AsMesh();
    mB->meshRenderer.asset = mA->meshRenderer.asset;

    // Slot-Materialien pro Instanz setzen (Slot 0, weil Würfel 1 Surface hat)
    Engine::engine->GetOM().SetSlotMaterial(mA, 0, matA);
    Engine::engine->GetOM().SetSlotMaterial(mB, 0, matB);

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();

        const float dt = static_cast<float>(Timer::GetDeltaTime());

        Engine::TurnEntity(cubeB, 0.0f, 100.0f * dt, 0.0f);

        Engine::Cls(0, 64, 128);

        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    return 0;
}