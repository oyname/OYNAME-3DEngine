// SpecularTest.cpp
// Dreht einen Würfel und bewegt ein Pointlight – Specular-Highlight muss wandern.

#include <Windows.h>
#include "gidx.h"

LPENTITY g_cubeMesh = nullptr;
LPENTITY g_camera = nullptr;
LPENTITY g_pointLight = nullptr;

void CreateCube(LPENTITY* mesh, LPMATERIAL material);

int main()
{
    Engine::Graphics(1024, 768);

    LPTEXTURE tex = nullptr;
    Engine::LoadTexture(&tex, L"..\\media\\color1.png");

    Engine::CreateCamera(&g_camera);
    Engine::PositionEntity(g_camera, 0.0f, 10.0f, -50.0f);
    Engine::RotateEntity(g_camera, 10.0f, 0.0f, 0.0f);

    // Material mit hohem Shininess – muss sichtbaren Highlight erzeugen
    LPMATERIAL material = nullptr;
    Engine::CreateMaterial(&material);
    Engine::MaterialTexture(material, tex);
    Engine::MaterialShininess(material, 64.0f);

    CreateCube(&g_cubeMesh, material);
    Engine::PositionEntity(g_cubeMesh, 0.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(g_cubeMesh, 8.0f, 8.0f, 8.0f);

    // Schwaches Ambient-Directional damit man die Oberfläche sieht
    LPENTITY dirLight = nullptr;
    Engine::CreateLight(&dirLight, D3DLIGHT_DIRECTIONAL);
    Engine::RotateEntity(dirLight, 45.0f, -45.0f, 0.0f);
    Engine::LightColor(dirLight, 0.9f, 0.3f, 0.3f);
    Engine::SetDirectionalLight(dirLight);
    Engine::SetAmbientColor(0.3f, 0.5f, 0.1f);

    // Pointlight das sich im Kreis bewegt → Specular-Highlight wandert sichtbar
    Engine::CreateLight(&g_pointLight, D3DLIGHT_POINT);
    Engine::LightColor(g_pointLight, 1.0f, 1.0f, 1.0f);

    Engine::SetVSync(1);

    float angle = 0.0f;
    const float speed = 60.0f;

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Core::GetDeltaTime());

        // Würfel dreht sich
        Engine::TurnEntity(g_cubeMesh, speed * dt, speed * dt * 0.7f, 0.0f);

        // Pointlight kreist um den Würfel
        angle += dt * 1.5f;
        float lx = sinf(angle) * 20.0f;
        float lz = cosf(angle) * 20.0f;
        Engine::PositionEntity(g_pointLight, lx, 10.0f, lz);

        Engine::Cls(0, 32, 64);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    return 0;
}

void CreateCube(LPENTITY* mesh, LPMATERIAL material)
{
    LPSURFACE s = nullptr;
    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&s, (*mesh));

    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 1);
    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 0);
    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 1);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 0);

    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 1);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 0);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 1);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 0);

    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 1);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 0);
    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 1);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 0);

    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 1);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 0);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 1);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 0);

    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 1);
    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 1);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 0);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 0);

    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 1);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 1);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0, 0);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1, 0);

    Engine::AddTriangle(s, 0, 1, 2); Engine::AddTriangle(s, 3, 2, 1);
    Engine::AddTriangle(s, 6, 5, 4); Engine::AddTriangle(s, 6, 7, 5);
    Engine::AddTriangle(s, 8, 9, 10); Engine::AddTriangle(s, 10, 9, 11);
    Engine::AddTriangle(s, 14, 13, 12); Engine::AddTriangle(s, 14, 15, 13);
    Engine::AddTriangle(s, 16, 17, 18); Engine::AddTriangle(s, 18, 17, 19);
    Engine::AddTriangle(s, 21, 22, 23); Engine::AddTriangle(s, 22, 21, 20);

    Engine::FillBuffer(s);
}