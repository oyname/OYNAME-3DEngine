#include "gidx.h"

LPENTITY g_quadMesh = nullptr;
LPENTITY quad_camera = nullptr;

void CreateQuad(LPENTITY* mesh, MATERIAL* material);

int main()
{
    bool windowed = true;

    windowed == true ? Engine::Graphics(1200, 650) : Engine::Graphics(1980, 1080, false);

    // Textur laden
    LPTEXTURE texBrick = nullptr; Engine::LoadTexture(&texBrick, L"..\\media\\dx.bmp");
    LPTEXTURE texFace = nullptr; Engine::LoadTexture(&texFace, L"..\\media\\face.bmp");
    LPTEXTURE texBricks = nullptr; Engine::LoadTexture(&texBricks, L"..\\media\\bricks.bmp");

    // Kamera erstellen
    Engine::CreateCamera(&quad_camera);
    Engine::PositionEntity(quad_camera, 0.0f, 0.0f, -5.0f);
    Engine::RotateEntity(quad_camera, 0.0f, 0.0f, 0.0f);

    // Material mit Textur
    LPMATERIAL material = nullptr;
    Engine::CreateMaterial(&material);
    Engine::MaterialTexture(material, texBrick);
    
    // Flaeche erstellen
    Engine::CreateMesh(&g_quadMesh);
    CreateQuad(&g_quadMesh, material);

    LPENTITY g_directionalLight = nullptr;
    Engine::CreateLight(&g_directionalLight, D3DLIGHT_DIRECTIONAL);  // ← DIRECTIONAL!
    Engine::PositionEntity(g_directionalLight, 0.0f, 0.0f, 0.0f);
    Engine::LightColor(g_directionalLight, 1.0f, 1.0f, 1.0f);

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) 
    {
        // Rendering
        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();
    }

    return 0;
}

void CreateQuad(LPENTITY* mesh, MATERIAL* material)
{
    LPSURFACE quad = NULL;

    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&quad, (*mesh));

    // Definition der Eckpunkte f�r jede Seite des W�rfels
    Engine::AddVertex(quad, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(quad, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(quad, 224, 224, 224);
    Engine::AddVertex(quad, -1.0f, 1.0f, -1.0f);  Engine::VertexNormal(quad, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(quad, 224, 224, 224);
    Engine::AddVertex(quad, 1.0f, -1.0f, -1.0f);  Engine::VertexNormal(quad, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(quad, 224, 224, 224);
    Engine::AddVertex(quad, 1.0f, 1.0f, -1.0f);   Engine::VertexNormal(quad, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(quad, 224, 224, 224);

    Engine::VertexTexCoord(quad, 0.0f, 1.0f);
    Engine::VertexTexCoord(quad, 0.0f, 0.0f);
    Engine::VertexTexCoord(quad, 1.0f, 1.0f);
    Engine::VertexTexCoord(quad, 1.0f, 0.0f);

    // Definition der Dreiecke fuer jede Seite des W�rfels
    Engine::AddTriangle(quad, 0, 1, 2);
    Engine::AddTriangle(quad, 3, 2, 1);

    Engine::FillBuffer(quad);
}