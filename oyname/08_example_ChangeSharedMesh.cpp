#include "gidx.h"
#include "geometry.h"

// Demonstriert:
// - Ein Mesh wird normal erzeugt (inkl. internem MeshAsset)
// - Ein zweites Mesh teilt dieses MeshAsset
// - Mit Leertaste wird das Mesh von cube2 durch ein drittes Mesh ersetzt
// - Die Materialien / Texturen der Instanzen bleiben getrennt


int main()
{
    Engine::Graphics(1200, 650, true);

    // -------------------------------------------------
    // Texturen
    // -------------------------------------------------
    LPTEXTURE texA = nullptr;
    LPTEXTURE texB = nullptr;
    Engine::LoadTexture(&texA, L"..\\media\\face.bmp");
    Engine::LoadTexture(&texB, L"..\\media\\dx.bmp");

    // -------------------------------------------------
    // Materialien
    // -------------------------------------------------
    LPMATERIAL matA = nullptr;
    LPMATERIAL matB = nullptr;

    Engine::CreateMaterial(&matA);
    Engine::CreateMaterial(&matB);

    Engine::MaterialSetAlbedo(matA, texA);
    Engine::MaterialSetAlbedo(matB, texB);

    Engine::MaterialRoughness(matA, 0.6f);
    Engine::MaterialMetallic(matA, 0.0f);

    Engine::MaterialRoughness(matB, 0.6f);
    Engine::MaterialMetallic(matB, 0.0f);

    // -------------------------------------------------
    // Licht
    // -------------------------------------------------
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::TurnEntity(light, 45, 0, 0);
    Engine::LightColor(light, 1, 1, 1);
    Engine::SetDirectionalLight(light);
    Engine::SetAmbientColor(0.2f, 0.2f, 0.2f);

    // -------------------------------------------------
    // Kamera
    // -------------------------------------------------
    LPENTITY cam = nullptr;
    Engine::CreateCamera(&cam);
    Engine::PositionEntity(cam, 0, 0, -8);
    Engine::SetCamera(cam);

    // -------------------------------------------------
    // Cube #1
    // -------------------------------------------------
    LPENTITY cube1 = nullptr;
    CreateCube(&cube1, matA);
    Engine::PositionEntity(cube1, -2.0f, 0.0f, 5.0f);
    Engine::SetSlotMaterial(cube1, 0, matA);

    // -------------------------------------------------
    // Cube #2
    // startet als geteiltes MeshAsset von cube1
    // -------------------------------------------------
    LPENTITY cube2 = nullptr;
    Engine::CreateMesh(&cube2);
    Engine::ShareMeshAsset(cube1, cube2);
    Engine::SetSlotMaterial(cube2, 0, matB);
    Engine::PositionEntity(cube2, 2.0f, 0.0f, 5.0f);

    // -------------------------------------------------
    // Mesh #3
    // anderes MeshAsset als Ersatz
    // -------------------------------------------------
    LPENTITY cube3 = nullptr;
    CreateSphere(&cube3, matB);
    Engine::PositionEntity(cube3, 0.0f, -0.0f, 0.0f); // aus dem Sichtbereich parken

    Engine::DebugPrintScene();

    bool spaceWasDown = false;
    bool replaced = false;

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();

        const bool spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;

        // Einmaliger Tastendruck
        if (spaceDown && !spaceWasDown && !replaced)
        {
            // cube2 bekommt jetzt das MeshAsset von cube3 statt von cube1
            Engine::ShareMeshAsset(cube3, cube2);
            Engine::SetSlotMaterial(cube2, 0, matB);

            replaced = true;
            Engine::DebugPrintScene();
        }

        spaceWasDown = spaceDown;

        Engine::TurnEntity(cube1, 0.2f, 0.4f, 0.0f);
        Engine::TurnEntity(cube2, -0.2f, -0.35f, 0.0f);

        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    return 0;
}