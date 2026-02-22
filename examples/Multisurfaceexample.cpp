// MultiSurfaceExample.cpp
// Demonstriert: Ein Mesh, 6 Surfaces, 3 Materialien.
// Jede Würfelseite ist eine eigene Surface mit eigenem Material.
//
// WICHTIG: Jede Seite hat exakt dieselbe Vertex-Definition und Winding-Order
// wie das Original-CreateCube – nur aufgeteilt auf separate Surfaces.

#include <Windows.h>
#include "gidx.h"

LPENTITY g_cubeMesh = nullptr;
LPENTITY g_plateMesh = nullptr;
LPENTITY g_camera = nullptr;
LPENTITY g_directionalLight = nullptr;

void CreateMultiMaterialCube(LPENTITY* mesh,
    LPMATERIAL matFront,   // Vorne/Hinten
    LPMATERIAL matSide,    // Links/Rechts
    LPMATERIAL matTopBot)  // Oben/Unten
{
    Engine::CreateMesh(mesh);

    LPSURFACE s = nullptr;

    // ---- Hinten (-Z)  winding: (0,1,2) (3,2,1) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 0, 1, 2);
    Engine::AddTriangle(s, 3, 2, 1);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matFront);

    // ---- Vorne (+Z)  winding: (2,1,0) (2,3,1) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 2, 1, 0);
    Engine::AddTriangle(s, 2, 3, 1);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matFront);

    // ---- Links (-X)  winding: (0,1,2) (2,1,3) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 0, 1, 2);
    Engine::AddTriangle(s, 2, 1, 3);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matSide);

    // ---- Rechts (+X)  winding: (2,1,0) (2,3,1) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 2, 1, 0);
    Engine::AddTriangle(s, 2, 3, 1);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matSide);

    // ---- Unten (-Y)  winding: (0,1,2) (2,1,3) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 0, 1, 2);
    Engine::AddTriangle(s, 2, 1, 3);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matTopBot);

    // ---- Oben (+Y)  winding: (1,2,3) (2,1,0) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 1, 2, 3);
    Engine::AddTriangle(s, 2, 1, 0);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matTopBot);
}

int main()
{
    Engine::Graphics(1024, 768);

    LPTEXTURE texBrick = nullptr; Engine::LoadTexture(&texBrick, L"..\\media\\dx.bmp");
    LPTEXTURE texFace = nullptr; Engine::LoadTexture(&texFace, L"..\\media\\face.bmp");
    LPTEXTURE texBricks = nullptr; Engine::LoadTexture(&texBricks, L"..\\media\\color3.png");

    LPMATERIAL matFront = nullptr;
    Engine::CreateMaterial(&matFront);
    Engine::MaterialTexture(matFront, texBrick);

    LPMATERIAL matSide = nullptr;
    Engine::CreateMaterial(&matSide);
    Engine::MaterialTexture(matSide, texFace);

    LPMATERIAL matTopBot = nullptr;
    Engine::CreateMaterial(&matTopBot);
    Engine::MaterialTexture(matTopBot, texBricks);

    CreateMultiMaterialCube(&g_cubeMesh, matFront, matSide, matTopBot);
    Engine::PositionEntity(g_cubeMesh, 0.0f, 20.0f, 0.0f);
    Engine::ScaleEntity(g_cubeMesh, 8.0f, 8.0f, 8.0f);

    // Bodenplatte – nutzt matTopBot → landet im selben Queue-Bucket wie Oben/Unten
    Engine::CreateMesh(&g_plateMesh, matTopBot);
    {
        LPSURFACE p = nullptr;
        Engine::CreateSurface(&p, g_plateMesh);
        Engine::AddVertex(p, -1, 0, -1); Engine::VertexNormal(p, 0, 1, 0); Engine::VertexColor(p, 200, 200, 200); Engine::VertexTexCoord(p, 0.0f, 1.0f);
        Engine::AddVertex(p, 1, 0, -1); Engine::VertexNormal(p, 0, 1, 0); Engine::VertexColor(p, 200, 200, 200); Engine::VertexTexCoord(p, 1.0f, 1.0f);
        Engine::AddVertex(p, -1, 0, 1); Engine::VertexNormal(p, 0, 1, 0); Engine::VertexColor(p, 200, 200, 200); Engine::VertexTexCoord(p, 0.0f, 0.0f);
        Engine::AddVertex(p, 1, 0, 1); Engine::VertexNormal(p, 0, 1, 0); Engine::VertexColor(p, 200, 200, 200); Engine::VertexTexCoord(p, 1.0f, 0.0f);
        Engine::AddTriangle(p, 1, 2, 3);
        Engine::AddTriangle(p, 2, 1, 0);
        Engine::FillBuffer(p);
    }
    Engine::PositionEntity(g_plateMesh, 0.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(g_plateMesh, 100.0f, 1.0f, 100.0f);

    Engine::CreateCamera(&g_camera);
    Engine::PositionEntity(g_camera, 0.0f, 30.0f, -90.0f);
    Engine::RotateEntity(g_camera, 10.0f, 0.0f, 0.0f);

    Engine::CreateLight(&g_directionalLight, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(g_directionalLight, 20.0f, 60.0f, -20.0f);
    Engine::RotateEntity(g_directionalLight, 45.0f, -45.0f, 0.0f);
    Engine::LightColor(g_directionalLight, 0.8f, 0.8f, 0.8f);
    Engine::SetDirectionalLight(g_directionalLight);
    Engine::SetAmbientColor(0.2f, 0.2f, 0.2f);
    Engine::SetVSync(1);

    const float speed = 80.0f;

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Core::GetDeltaTime());
        Engine::TurnEntity(g_cubeMesh, speed * dt, speed * dt * 0.7f, 0.0f);
        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();
        Core::EndFrame();
    }

    return 0;
}