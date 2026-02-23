#include <Windows.h>
#include "gidx.h"
#include "RenderLayers.h"

LPENTITY g_plateMesh = nullptr;
LPENTITY g_cubeMesh = nullptr;
LPENTITY g_cubeMesh2 = nullptr;
LPENTITY g_camera = nullptr;
LPENTITY g_redLight = nullptr;
LPENTITY g_blueLight = nullptr;
LPENTITY g_directionalLight = nullptr;

void CreateCube(LPENTITY* mesh, MATERIAL* material);

int main()
{
    Engine::Graphics(1024, 768);

    // Texturen
    LPTEXTURE brick = nullptr; Engine::LoadTexture(&brick, L"..\\media\\color1.png");
    LPTEXTURE dxlogo = nullptr; Engine::LoadTexture(&dxlogo, L"..\\media\\bricks.bmp");
    LPTEXTURE face = nullptr; Engine::LoadTexture(&face, L"..\\media\\color3.png");

    // Kamera
    Engine::CreateCamera(&g_camera);
    Engine::PositionEntity(g_camera, 0.0f, 30.0f, -90.0f);
    Engine::RotateEntity(g_camera, 10.0f, 0.0f, 0.0f);

    // Kamera sieht standardmäßig alle Layer
    Engine::CameraCullMask(g_camera, LAYER_ALL);

    // Würfel 1 – Layer: DEFAULT
    LPMATERIAL whiteMaterial = nullptr;
    Engine::CreateMaterial(&whiteMaterial);
    Engine::MaterialTexture(whiteMaterial, brick);
    CreateCube(&g_cubeMesh, whiteMaterial);
    Engine::PositionEntity(g_cubeMesh, 0.0f, 35.0f, 0.0f);
    Engine::ScaleEntity(g_cubeMesh, 8.0f, 8.0f, 8.0f);
    Engine::EntityLayer(g_cubeMesh, LAYER_DEFAULT);          // Taste 1 schaltet ihn aus

    // Würfel 2 – Layer: LAYER_FX
    LPMATERIAL faceMaterial = nullptr;
    Engine::CreateMaterial(&faceMaterial);
    Engine::MaterialTexture(faceMaterial, face);
    CreateCube(&g_cubeMesh2, faceMaterial);
    Engine::PositionEntity(g_cubeMesh2, 0.0f, 6.0f, 0.0f);
    Engine::ScaleEntity(g_cubeMesh2, 5.0f, 5.0f, 5.0f);
    Engine::EntityLayer(g_cubeMesh2, LAYER_FX);              // Taste 2 schaltet ihn aus

    // Platte – Layer: DEFAULT | LAYER_REFLECTION
    LPMATERIAL grayMaterial = nullptr;
    Engine::CreateMaterial(&grayMaterial);
    Engine::MaterialTexture(grayMaterial, dxlogo);
    CreateCube(&g_plateMesh, grayMaterial);
    Engine::PositionEntity(g_plateMesh, 0.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(g_plateMesh, 100.0f, 1.0f, 100.0f);
    Engine::EntityLayer(g_plateMesh, LAYER_DEFAULT | LAYER_REFLECTION);

    // Lichter
    Engine::CreateLight(&g_directionalLight, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(g_directionalLight, 20.0f, 60.0f, -20.0f);
    Engine::RotateEntity(g_directionalLight, 45.0f, -45.0f, 0.0f);
    Engine::LightColor(g_directionalLight, 0.1f, 0.1f, 0.1f);

    Engine::CreateLight(&g_redLight, D3DLIGHT_POINT);
    Engine::PositionEntity(g_redLight, 20.0f, 15.0f, 0.0f);
    Engine::LightColor(g_redLight, 1.0f, 0.3f, 0.3f);

    Engine::CreateLight(&g_blueLight, D3DLIGHT_POINT);
    Engine::PositionEntity(g_blueLight, -20.0f, 15.0f, 0.0f);
    Engine::LightColor(g_blueLight, 0.3f, 0.3f, 1.0f);

    Engine::SetDirectionalLight(g_directionalLight);
    Engine::SetAmbientColor(0.5f, 0.5f, 0.7f);
    Engine::SetVSync(1);

    const float speed = 100.0f;

    // Zustand der Tasten: Debounce damit nicht jeder Frame schaltet
    bool key1Last = false;
    bool key2Last = false;
    bool key3Last = false;
    bool key4Last = false;

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Core::GetDeltaTime());

        // ---- Lichtsteuerung ----
        if (GetAsyncKeyState(VK_UP) & 0x8000) Engine::MoveEntity(g_redLight, 0.0f, 0.0f, 50.0f * dt);
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) Engine::MoveEntity(g_redLight, 0.0f, 0.0f, -50.0f * dt);
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Engine::MoveEntity(g_redLight, 50.0f * dt, 0.0f, 0.0f);
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) Engine::MoveEntity(g_redLight, -50.0f * dt, 0.0f, 0.0f);

        // ---- Layer-Steuerung (Tasten 1-4, Debounce) ----

        // [1] – LAYER_DEFAULT in cullMask der Kamera toggeln
        //       Würfel 1 und Platte verschwinden / erscheinen
        bool key1 = (GetAsyncKeyState('1') & 0x8000) != 0;
        if (key1 && !key1Last)
        {
            uint32_t mask = Engine::CameraCullMask(g_camera);
            mask ^= LAYER_DEFAULT;                           // Bit flippen
            Engine::CameraCullMask(g_camera, mask);
        }
        key1Last = key1;

        // [2] – LAYER_FX in cullMask toggeln
        //       Würfel 2 verschwindet / erscheint
        bool key2 = (GetAsyncKeyState('2') & 0x8000) != 0;
        if (key2 && !key2Last)
        {
            uint32_t mask = Engine::CameraCullMask(g_camera);
            mask ^= LAYER_FX;
            Engine::CameraCullMask(g_camera, mask);
        }
        key2Last = key2;

        // [3] – ShowEntity toggeln auf Würfel 1
        //       Objekt unsichtbar, aber Update/Rotation läuft weiter
        bool key3 = (GetAsyncKeyState('3') & 0x8000) != 0;
        if (key3 && !key3Last)
        {
            bool nowVisible = !Engine::EntityVisible(g_cubeMesh);
            Engine::ShowEntity(g_cubeMesh, nowVisible);
        }
        key3Last = key3;

        // [4] – EntityActive toggeln auf Würfel 2
        //       Objekt komplett eingefroren: keine Rotation, kein Rendering
        bool key4 = (GetAsyncKeyState('4') & 0x8000) != 0;
        if (key4 && !key4Last)
        {
            bool nowActive = !Engine::EntityActive(g_cubeMesh2);
            Engine::EntityActive(g_cubeMesh2, nowActive);
        }
        key4Last = key4;

        // ---- Update & Render ----
        Engine::TurnEntity(g_cubeMesh, speed * dt, speed * dt, 0.0f);

        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();
        Core::EndFrame();
    }

    return 0;
}

void CreateCube(LPENTITY* mesh, MATERIAL* material)
{
    LPSURFACE wuerfel = NULL;

    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&wuerfel, (*mesh));

    // Definition der Eckpunkte f�r jede Seite des W�rfels
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f);   Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f);  Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f);   Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f);   Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f);    Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f);  Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f);  Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f);   Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f);  Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f);   Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f);   Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f);    Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f);  Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f);  Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f);   Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f);  Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f);   Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f);   Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f);    Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);

    // Definition der Dreiecke f�r jede Seite des W�rfels
    Engine::AddTriangle(wuerfel, 0, 1, 2);
    Engine::AddTriangle(wuerfel, 3, 2, 1);

    Engine::AddTriangle(wuerfel, 6, 5, 4);
    Engine::AddTriangle(wuerfel, 6, 7, 5);

    Engine::AddTriangle(wuerfel, 8, 9, 10);
    Engine::AddTriangle(wuerfel, 10, 9, 11);

    Engine::AddTriangle(wuerfel, 14, 13, 12);
    Engine::AddTriangle(wuerfel, 14, 15, 13);

    Engine::AddTriangle(wuerfel, 16, 17, 18);
    Engine::AddTriangle(wuerfel, 18, 17, 19);

    Engine::AddTriangle(wuerfel, 21, 22, 23);
    Engine::AddTriangle(wuerfel, 22, 21, 20);

    Engine::FillBuffer(wuerfel);
}