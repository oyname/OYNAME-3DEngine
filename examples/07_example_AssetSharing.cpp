// 07_example_AssetSharing.cpp
//
// MeshAsset-Sharing Demo
// ----------------------
// Zwei Wuerfel-Entities teilen sich dasselbe MeshAsset (identische Geometrie,
// ein einziger GPU-Buffer). Jede Instanz hat ein eigenes Material, einen
// eigenen Transform und verhält sich vollständig unabhängig.
//
// Kernpunkt: Engine::ShareMeshAsset() tauscht das automatisch angelegte
// Asset von cubeB gegen das Asset von cubeA, loescht das Waisen-Asset
// sauber aus dem ObjectManager und vermeidet so Memory-Leak und
// Doppeldelete beim Shutdown.

#include "gidx.h"
#include "geometry.h"

int main()
{
    Engine::Graphics(1280, 720);

    // -------------------------------------------------
    // Kamera
    // -------------------------------------------------
    LPENTITY cam = nullptr;
    Engine::CreateCamera(&cam);
    Engine::PositionEntity(cam, 0.0f, 2.0f, -8.0f);
    Engine::LookAt(cam, 0.0f, 1.0f, 0.0f);
    Engine::SetCamera(cam);

    // -------------------------------------------------
    // Licht
    // -------------------------------------------------
    LPENTITY sun = nullptr;
    Engine::CreateLight(&sun, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(sun, 10.0f, 10.0f, -10.0f);
    Engine::LookAt(sun, 0.0f, 0.0f, 0.0f);
    Engine::LightColor(sun, 2.0f, 2.0f, 2.0f);
    Engine::SetDirectionalLight(sun);
    Engine::SetAmbientColor(0.2f, 0.2f, 0.2f);

    // -------------------------------------------------
    // Texturen
    // -------------------------------------------------
    LPTEXTURE texA = nullptr;
    Engine::LoadTexture(&texA, L"..\\media\\dx.bmp");

    LPTEXTURE texB = nullptr;
    Engine::LoadTexture(&texB, L"..\\media\\face.bmp");

    // -------------------------------------------------
    // Materialien (unterschiedliche Albedo-Textur)
    // -------------------------------------------------
    LPMATERIAL matA = nullptr;
    Engine::CreateMaterial(&matA);
    Engine::MaterialRoughness(matA, 0.6f);
    Engine::MaterialMetallic(matA, 0.0f);
    Engine::MaterialSetAlbedo(matA, texA);

    LPMATERIAL matB = nullptr;
    Engine::CreateMaterial(&matB);
    Engine::MaterialRoughness(matB, 0.6f);
    Engine::MaterialMetallic(matB, 0.0f);
    Engine::MaterialSetAlbedo(matB, texB);

    // -------------------------------------------------
    // Entity A: Wuerfel erzeugen
    // CreateCube legt Mesh + MeshAsset + Surface(Slot 0) + GPU-Buffer an.
    // -------------------------------------------------
    LPENTITY cubeA = nullptr;
    CreateCube(&cubeA, matA);
    Engine::PositionEntity(cubeA, -1.5f, 1.0f, 0.0f);

    // -------------------------------------------------
    // Entity B: leeres Mesh, teilt das Asset von A
    // ShareMeshAsset() loescht das automatisch angelegte Asset von cubeB
    // und setzt stattdessen das Asset von cubeA. Kein Leak, kein Doppeldelete.
    // -------------------------------------------------
    LPENTITY cubeB = nullptr;
    Engine::CreateMesh(&cubeB);
    Engine::PositionEntity(cubeB, 1.5f, 1.0f, 0.0f);

    Engine::ShareMeshAsset(cubeA, cubeB);

    // Pro Instanz eigenes Material fuer Slot 0 setzen.
    Engine::SetSlotMaterial(cubeA, 0, matA);
    Engine::SetSlotMaterial(cubeB, 0, matB);

    // -------------------------------------------------
    // Main Loop
    // -------------------------------------------------
    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Timer::GetDeltaTime());

        // Nur cubeB dreht sich – cubeA steht still.
        // Beide nutzen denselben GPU-Buffer.
        Engine::TurnEntity(cubeB, 0.0f, 100.0f * dt, 0.0f);

        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    return 0;
}
