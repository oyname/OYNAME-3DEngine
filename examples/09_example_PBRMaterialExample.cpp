// game.cpp
#include "gidx.h"
#include "core.h"

void CreateCube(LPENTITY* mesh, MATERIAL* material);

int main()
{
    // Engine ist schon initialisiert (WinMain hat Core::Init + Core::CreateEngine aufgerufen).
    // Hier nur noch Graphics-Modus setzen:
    Engine::Graphics(1024, 768);

    // -------------------------------------------------
    // Kamera
    // -------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 0.0f, -15.0f);

    // -------------------------------------------------
    // Licht
    // -------------------------------------------------
    LPENTITY directionalLight = nullptr;
    Engine::CreateLight(&directionalLight, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(directionalLight, 10.0f, 0.0f, 0.0f);
    Engine::LookAt(directionalLight, 1.0f, 0.0f, 3.0f);
    Engine::LightColor(directionalLight, 1.8f, 1.8f, 1.8f);
    Engine::SetDirectionalLight(directionalLight);
    Engine::SetAmbientColor(0.3f, 0.3f, 0.3f);

    // -------------------------------------------------
    // Textur laden
    // -------------------------------------------------
    LPTEXTURE faceTex = nullptr;
    LPTEXTURE albedoTex = nullptr;
    LPTEXTURE normalTex = nullptr;
    LPTEXTURE detailTex = nullptr;
    Engine::LoadTexture(&faceTex, L"..\\media\\engine.png");
    Engine::LoadTexture(&albedoTex, L"..\\media\\albedo.png");
    Engine::LoadTexture(&normalTex, L"..\\media\\normal.png");
    Engine::LoadTexture(&detailTex, L"..\\media\\albedo_orage.png");

    // -------------------------------------------------
    // Material A erstellen
    // -------------------------------------------------
    LPMATERIAL matCube = nullptr;
    Engine::CreateMaterial(&matCube);

    Engine::MaterialUsePBR(matCube, true);

    Engine::MaterialTexture(matCube, albedoTex, 0);
    Engine::MaterialTexture(matCube, normalTex, 1);  // <-- NORMAL ist Slot 2
    Engine::MaterialTexture(matCube, detailTex, 3);  // <-- Detailmap ist Slot 4 


    Engine::MaterialBlendMode(matCube, 2);        // Multiply x2
    Engine::MaterialBlendFactor(matCube, 0.99f);   // Mischstaerke 0..1

    // Basisfarbe
    Engine::MaterialColor(matCube, 1.0f, 1.0f, 1.0f, 1.0f);

    // PBR Parameter
    Engine::MaterialMetallic(matCube, 1.0f);
    Engine::MaterialRoughness(matCube, 0.5f);
    Engine::MaterialNormalScale(matCube, 0.3f);
    //Engine::MaterialOcclusionStrength(matCube, 0.0f);

    // Emissive (leichtes Glühen)
    //Engine::MaterialEmissiveColor(matCube, 1.0f, 0.3f, 0.0f, 1.5f);

    // -------------------------------------------------
    // Material B erstellen
    // -------------------------------------------------
    LPMATERIAL matBCube = nullptr;
    Engine::CreateMaterial(&matBCube);


    Engine::MaterialTexture(matBCube, faceTex, 0);
    Engine::MaterialTexture(matBCube, detailTex, 3);  // <-- Detailmap ist Slot 4 
    // Basisfarbe
    Engine::MaterialColor(matCube, 1.0f, 1.0f, 1.0f, 1.0f);
    Engine::MaterialMetallic(matBCube, 2.0f);
    Engine::MaterialRoughness(matBCube, 0.05f);

    Engine::MaterialUsePBR(matBCube, true);

    // -------------------------------------------------
    // Objekt erstellen
    // -------------------------------------------------
    LPENTITY cube;
    CreateCube(&cube, matBCube);
    Engine::PositionEntity(cube, -3.0f, 0.0f, 10.0f);
    Engine::ScaleEntity(cube, 2.0f, 2.0f, 2.0f);

    LPENTITY cube2;
    CreateCube(&cube2, matCube);
    Engine::PositionEntity(cube2, 3.0f, 0.0f, 8.0f);
    Engine::ScaleEntity(cube2, 1.0f, 1.0f, 1.0f);

    // Set Parent
    Engine::SetEntityParent(cube2, cube);

    while (Windows::MainLoop())
    {
        Core::BeginFrame(); // liefert DeltaTime/FPS/FrameCount über Core
        
        float dt = (float)Timer::GetDeltaTime();

        // Rotation
        //Engine::LookAt(camera, Engine::EntityPosition(cube2));
        Engine::LookAt(directionalLight, Engine::EntityPosition(cube2));

        Engine::RotateEntity(cube2, 45.0f * dt, 45.0f * dt, 45.0f * dt, Space::World);
        Engine::RotateEntity(cube, -10.0f * dt, -45.0f * dt, -2.0f * dt);

        Engine::Cls(0, 64, 128);

        Engine::UpdateWorld();

        HRESULT hr = Engine::RenderWorld();
        if (FAILED(hr))
            Debug::Log("FEHLER: RenderWorld");

        hr = Engine::Flip();
        if (FAILED(hr))
            Debug::Log("FEHLER: Flip");

        Core::EndFrame();
    }

    return 0;
}

void CreateCube(LPENTITY* mesh, MATERIAL* material)
{
    LPSURFACE wuerfel = NULL;

    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&wuerfel, (*mesh));

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    // Back (verts 0-3)
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    // Front (verts 4-7)
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    // Left (verts 8-11)
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    // Right (verts 12-15)
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    // Bottom (verts 16-19)
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    // Top (verts 20-23)
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);


    // Back (verts 0-3)
    Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f);
    // Front (verts 4-7)
    Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f);
    // Left (verts 8-11)
    Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f);
    // Right (verts 12-15)
    Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f);
    // Bottom (verts 16-19)
    Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f);
    // Top (verts 20-23)
    Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f);

    Engine::AddTriangle(wuerfel, 0, 1, 2); Engine::AddTriangle(wuerfel, 3, 2, 1);
    Engine::AddTriangle(wuerfel, 6, 5, 4); Engine::AddTriangle(wuerfel, 6, 7, 5);
    Engine::AddTriangle(wuerfel, 8, 9, 10); Engine::AddTriangle(wuerfel, 10, 9, 11);
    Engine::AddTriangle(wuerfel, 14, 13, 12); Engine::AddTriangle(wuerfel, 14, 15, 13);
    Engine::AddTriangle(wuerfel, 16, 17, 18); Engine::AddTriangle(wuerfel, 18, 17, 19);
    Engine::AddTriangle(wuerfel, 21, 22, 23); Engine::AddTriangle(wuerfel, 22, 21, 20);

    Engine::FillBuffer(wuerfel);
}