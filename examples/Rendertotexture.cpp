#include "gidx.h"

static void CreateCube(LPENTITY* mesh, LPMATERIAL material);

void main(LPVOID)
{
    Engine::Graphics(1200, 650);

    LPTEXTURE texture = nullptr;
    Engine::LoadTexture(&texture, L"..\\media\\face.bmp");

    LPMATERIAL material = nullptr;
    Engine::CreateMaterial(&material);
    Engine::MaterialTexture(material, texture);

    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 0.0f, -10.0f);

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
    Engine::SetAmbientColor(0.1f, 0.1f, 0.1f);

    // Linker Würfel — dreht sich
    LPENTITY cubeLeft = nullptr;
    CreateCube(&cubeLeft, material);
    Engine::PositionEntity(cubeLeft, -4.0f, 0.0f, 10.0f);
    Engine::ScaleEntity(cubeLeft, 2.0f, 2.0f, 2.0f);
    Engine::EntityTexture(cubeLeft, texture);
    Engine::engine->GetOM().AddMeshToMaterial(material, dynamic_cast<Mesh*>(cubeLeft));

    // Rechter Würfel — steht still
    LPENTITY cubeRight = nullptr;
    CreateCube(&cubeRight, material);
    Engine::PositionEntity(cubeRight, 4.0f, 0.0f, 10.0f);
    Engine::ScaleEntity(cubeRight, 2.0f, 2.0f, 2.0f);
    Engine::EntityTexture(cubeRight, texture);
    Engine::engine->GetOM().AddMeshToMaterial(material, dynamic_cast<Mesh*>(cubeRight));

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Core::GetDeltaTime());

        Engine::TurnEntity(cubeLeft, 60.0f * dt, 80.0f * dt, 0.0f);

        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }
}

static void CreateCube(LPENTITY* mesh, LPMATERIAL material)
{
    LPSURFACE wuerfel = nullptr;

    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&wuerfel, (*mesh));

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f); Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f); Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f); Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::AddTriangle(wuerfel, 0, 1, 2); Engine::AddTriangle(wuerfel, 3, 2, 1);
    Engine::AddTriangle(wuerfel, 6, 5, 4); Engine::AddTriangle(wuerfel, 6, 7, 5);
    Engine::AddTriangle(wuerfel, 8, 9, 10); Engine::AddTriangle(wuerfel, 10, 9, 11);
    Engine::AddTriangle(wuerfel, 14, 13, 12); Engine::AddTriangle(wuerfel, 14, 15, 13);
    Engine::AddTriangle(wuerfel, 16, 17, 18); Engine::AddTriangle(wuerfel, 18, 17, 19);
    Engine::AddTriangle(wuerfel, 21, 22, 23); Engine::AddTriangle(wuerfel, 22, 21, 20);

    Engine::FillBuffer(wuerfel);
}