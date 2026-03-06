#include "gidx.h"
#include "geometry.h"


// Beispiel mit einem zweiten Shader. Der zweite Shader stellt das Objekt nur in Rot

int main()
{
    bool windowed = true;
    windowed == true ? Engine::Graphics(1200, 650) : Engine::Graphics(1980, 1080, false);

    // Neue Shader
    LPSHADER shader = nullptr;
    Engine::CreateShader(&shader, L"..\\shaders\\VertexShaderNeon.hlsl", "main", L"..\\shaders\\PixelShaderNeon.hlsl", "main", Engine::CreateVertexFlags(true, false, false, false, false));

    // Textur laden
    LPTEXTURE face = nullptr;
    Engine::LoadTexture(&face, L"..\\media\\color3.png");

    // Kamera erstellen
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 0.0f, -5.0f);
    Engine::RotateEntity(camera, 0.0f, 0.0f, 0.0f);

    // Material 1 mit Textur Standard-Shader
    LPMATERIAL material1 = nullptr;
    Engine::CreateMaterial(&material1);
    Engine::MaterialTexture(material1, face);

    // Material 2 mit dem neuen Shader
    LPMATERIAL material2 = nullptr;
    Engine::CreateMaterial(&material2, shader);

    // Qube erstellen
    LPENTITY Mesh1 = nullptr;
    CreateCube(&Mesh1, material1);
    Engine::PositionEntity(Mesh1, -2.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(Mesh1, 1.0f, 1.0f, 1.0f);

    // Zweiten Qube erstellen
    LPENTITY Mesh2 = nullptr;
    CreateCube(&Mesh2, material2);
    Engine::PositionEntity(Mesh2, 2.0f, 0.0f, 0.0f);

    // LICHT
    LPENTITY directionalLight = nullptr;
    Engine::CreateLight(&directionalLight, D3DLIGHT_POINT);
    Engine::PositionEntity(directionalLight, 0.0f, 0.0f, -10.0f);
    Engine::RotateEntity(directionalLight, 90.0f, 20.0f, 0.0f);
    Engine::LightColor(directionalLight, 1.0f, 1.0f, 1.0f);

    // 
    Engine::SetVSync(1);

    Engine::DebugPrintScene();

    const float speed = 100.0f;

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();

        const float dt = static_cast<float>(Timer::GetDeltaTime());

        if ((GetAsyncKeyState(VK_UP) & 0x8000))
        {
            Engine::MoveEntity(Mesh1, 0.0f, 0.0f, 5.0f * dt);
        }
        if ((GetAsyncKeyState(VK_DOWN) & 0x8000))
        {
            Engine::MoveEntity(Mesh1, 0.0f, 0.0f, -5.0f * dt);
        }
        if ((GetAsyncKeyState(VK_RIGHT) & 0x8000))
        {
            Engine::MoveEntity(Mesh1, 5.0f * dt, 0.0f, 0.0f);
        }
        if ((GetAsyncKeyState(VK_LEFT) & 0x8000))
        {
            Engine::MoveEntity(Mesh1, -5.0f * dt, 0.0f, 0.0f);
        }

        // Rendering
        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    return 0;
}

