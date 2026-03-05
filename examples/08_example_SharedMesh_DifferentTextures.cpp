#include "gidx.h"

// Demonstriert:
// - Ein MeshAsset (Geometrie) wird EINMAL gebaut.
// - Zwei Mesh-Entities referenzieren dasselbe Asset,
//   haben aber unterschiedliche Materialien/Texturen (Slot 0).

static void BuildCubeAsset(MeshAsset* asset)
{
    LPSURFACE s = nullptr;
    Engine::CreateSurface(&s, asset);

    // Positions + Normals + Color (wie der alte CreateCube, aber ohne CreateMesh)
    Engine::AddVertex(s, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(s, 0.0f, 0.0f, -1.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s, -1.0f,  1.0f, -1.0f); Engine::VertexNormal(s, 0.0f, 0.0f, -1.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f, -1.0f, -1.0f); Engine::VertexNormal(s, 0.0f, 0.0f, -1.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f,  1.0f, -1.0f); Engine::VertexNormal(s, 0.0f, 0.0f, -1.0f); Engine::VertexColor(s, 224, 224, 224);

    Engine::AddVertex(s, -1.0f, -1.0f,  1.0f); Engine::VertexNormal(s, 0.0f, 0.0f,  1.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s, -1.0f,  1.0f,  1.0f); Engine::VertexNormal(s, 0.0f, 0.0f,  1.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f, -1.0f,  1.0f); Engine::VertexNormal(s, 0.0f, 0.0f,  1.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f,  1.0f,  1.0f); Engine::VertexNormal(s, 0.0f, 0.0f,  1.0f); Engine::VertexColor(s, 224, 224, 224);

    Engine::AddVertex(s, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(s, -1.0f, 0.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s, -1.0f, -1.0f,  1.0f); Engine::VertexNormal(s, -1.0f, 0.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s, -1.0f,  1.0f, -1.0f); Engine::VertexNormal(s, -1.0f, 0.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s, -1.0f,  1.0f,  1.0f); Engine::VertexNormal(s, -1.0f, 0.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);

    Engine::AddVertex(s,  1.0f, -1.0f, -1.0f); Engine::VertexNormal(s, 1.0f, 0.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f, -1.0f,  1.0f); Engine::VertexNormal(s, 1.0f, 0.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f,  1.0f, -1.0f); Engine::VertexNormal(s, 1.0f, 0.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f,  1.0f,  1.0f); Engine::VertexNormal(s, 1.0f, 0.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);

    Engine::AddVertex(s, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(s, 0.0f, -1.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f, -1.0f, -1.0f); Engine::VertexNormal(s, 0.0f, -1.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s, -1.0f, -1.0f,  1.0f); Engine::VertexNormal(s, 0.0f, -1.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f, -1.0f,  1.0f); Engine::VertexNormal(s, 0.0f, -1.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);

    Engine::AddVertex(s, -1.0f,  1.0f, -1.0f); Engine::VertexNormal(s, 0.0f, 1.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f,  1.0f, -1.0f); Engine::VertexNormal(s, 0.0f, 1.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s, -1.0f,  1.0f,  1.0f); Engine::VertexNormal(s, 0.0f, 1.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);
    Engine::AddVertex(s,  1.0f,  1.0f,  1.0f); Engine::VertexNormal(s, 0.0f, 1.0f, 0.0f); Engine::VertexColor(s, 224, 224, 224);

    // UV1
    // Back
    Engine::VertexTexCoord(s, 0.0f, 1.0f); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::VertexTexCoord(s, 1.0f, 1.0f); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    // Front
    Engine::VertexTexCoord(s, 1.0f, 1.0f); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::VertexTexCoord(s, 0.0f, 1.0f); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    // Left
    Engine::VertexTexCoord(s, 1.0f, 1.0f); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::VertexTexCoord(s, 1.0f, 0.0f); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    // Right
    Engine::VertexTexCoord(s, 0.0f, 1.0f); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::VertexTexCoord(s, 0.0f, 0.0f); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    // Bottom
    Engine::VertexTexCoord(s, 0.0f, 0.0f); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::VertexTexCoord(s, 0.0f, 1.0f); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    // Top
    Engine::VertexTexCoord(s, 1.0f, 0.0f); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::VertexTexCoord(s, 1.0f, 1.0f); Engine::VertexTexCoord(s, 0.0f, 1.0f);

    // Indices
    Engine::AddTriangle(s, 0, 1, 2);  Engine::AddTriangle(s, 3, 2, 1);
    Engine::AddTriangle(s, 6, 5, 4);  Engine::AddTriangle(s, 6, 7, 5);
    Engine::AddTriangle(s, 8, 9, 10); Engine::AddTriangle(s, 10, 9, 11);
    Engine::AddTriangle(s, 14, 13, 12); Engine::AddTriangle(s, 14, 15, 13);
    Engine::AddTriangle(s, 16, 17, 18); Engine::AddTriangle(s, 18, 17, 19);
    Engine::AddTriangle(s, 21, 22, 23); Engine::AddTriangle(s, 22, 21, 20);

    Engine::FillBuffer(s);
}

int main()
{
    Engine::Graphics(1200, 650, true);

    // Texturen
    LPTEXTURE texA = nullptr;
    LPTEXTURE texB = nullptr;
    Engine::LoadTexture(&texA, L"..\\media\\face.bmp");
    Engine::LoadTexture(&texB, L"..\\media\\dx.bmp");

    // Materialien (unterschiedliche Texturen)
    LPMATERIAL matA = nullptr;
    LPMATERIAL matB = nullptr;
    Engine::CreateMaterial(&matA);
    Engine::CreateMaterial(&matB);
    Engine::MaterialTexture(matA, texA, 0);
    Engine::MaterialTexture(matB, texB, 0);

    // Licht
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::TurnEntity(light, 45, 0, 0);
    Engine::LightColor(light, 1, 1, 1);
    Engine::SetDirectionalLight(light);
    Engine::SetAmbientColor(0.2f, 0.2f, 0.2f);

    // Kamera
    LPENTITY cam = nullptr;
    Engine::CreateCamera(&cam);
    Engine::PositionEntity(cam, 0, 0, -8);
    Engine::SetCamera(cam);

    // Shared MeshAsset bauen
    MeshAsset* cubeAsset = nullptr;
    Engine::CreateMeshAsset(&cubeAsset);
    BuildCubeAsset(cubeAsset);

    // Cube #1
    LPENTITY cube1 = nullptr;
    Engine::CreateMesh(&cube1);
    Engine::SetMeshAsset(cube1, cubeAsset);
    Engine::SetMeshSlotMaterial(cube1, 0, matA);
    Engine::PositionEntity(cube1, -2.0f, 0.0f, 5.0f);

    // Cube #2 (selbe Geometrie, anderes Material)
    LPENTITY cube2 = nullptr;
    Engine::CreateMesh(&cube2);
    Engine::SetMeshAsset(cube2, cubeAsset);
    Engine::SetMeshSlotMaterial(cube2, 0, matB);
    Engine::PositionEntity(cube2, 2.0f, 0.0f, 5.0f);

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();

        // simple rotation, damit man sieht es sind 2 Instanzen
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
