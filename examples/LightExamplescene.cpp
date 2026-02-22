#include <Windows.h>
#include "gidx.h"

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
    // Engine init
    Engine::Graphics(1024, 768);

    // Texturen
    LPTEXTURE brick = nullptr;
    Engine::LoadTexture(&brick, L"..\\media\\color1.png");

    LPTEXTURE dxlogo = nullptr;
    Engine::LoadTexture(&dxlogo, L"..\\media\\bricks.bmp");

    LPTEXTURE face = nullptr;
    Engine::LoadTexture(&face, L"..\\media\\color3.png");

    // Kamera
    Engine::CreateCamera(&g_camera);
    Engine::PositionEntity(g_camera, 0.0f, 30.0f, -90.0f);
    Engine::RotateEntity(g_camera, 10.0f, 0.0f, 0.0f);

    // Wuerfel 1
    LPMATERIAL whiteMaterial = nullptr;
    Engine::CreateMaterial(&whiteMaterial);
    Engine::MaterialTexture(whiteMaterial, brick);

    CreateCube(&g_cubeMesh, whiteMaterial);  // CreateCube ruft intern CreateMesh auf
    Engine::PositionEntity(g_cubeMesh, 0.0f, 35.0f, 0.0f);
    Engine::ScaleEntity(g_cubeMesh, 8.0f, 8.0f, 8.0f);


    // Wuerfel 2
    LPMATERIAL faceMaterial = nullptr;
    Engine::CreateMaterial(&faceMaterial);
    Engine::MaterialTexture(faceMaterial, face);

    CreateCube(&g_cubeMesh2, faceMaterial);  // CreateCube ruft intern CreateMesh auf
    Engine::PositionEntity(g_cubeMesh2, 0.0f, 6.0f, 0.0f);
    Engine::ScaleEntity(g_cubeMesh2, 5.0f, 5.0f, 5.0f);

    // Platte
    LPMATERIAL grayMaterial = nullptr;
    Engine::CreateMaterial(&grayMaterial);
    Engine::MaterialTexture(grayMaterial, dxlogo);

    CreateCube(&g_plateMesh, grayMaterial);  // CreateCube ruft intern CreateMesh auf
    Engine::PositionEntity(g_plateMesh, 0.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(g_plateMesh, 100.0f, 1.0f, 100.0f);

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

    // Dieses Licht wirft Schatten (muss nach CreateLight aufgerufen werden)
    Engine::SetDirectionalLight(g_directionalLight);
    Engine::SetAmbientColor(0.5f, 0.5f, 0.7f);

    Engine::SetVSync(1);

    //Engine::PositionEntity(g_camera, 20.0f, 80.0f, -20.0f);
    //Engine::RotateEntity(g_camera, 45.0f, -45.0f, 0.0f);

    const float speed = 100.0f;

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();

        const float dt = static_cast<float>(Core::GetDeltaTime());

        Engine::Cls(0, 64, 128);

        if ((GetAsyncKeyState(VK_UP) & 0x8000)) {
            Engine::MoveEntity(g_redLight, 0.0f, 0.0f, 50.0f * dt);
        }
        if ((GetAsyncKeyState(VK_DOWN) & 0x8000)) {
            Engine::MoveEntity(g_redLight, 0.0f, 0.0f, -50.0f * dt);
        }
        if ((GetAsyncKeyState(VK_RIGHT) & 0x8000)) {
            Engine::MoveEntity(g_redLight, 50.0f * dt, 0.0f, 0.0f);
        }
        if ((GetAsyncKeyState(VK_LEFT) & 0x8000)) {
            Engine::MoveEntity(g_redLight, -50.0f * dt, 0.0f, 0.0f);
        }

        Engine::TurnEntity(g_cubeMesh, speed * dt, speed * dt, 0.0f);

        // Rendering
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