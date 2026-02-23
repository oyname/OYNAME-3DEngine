#include "gidx.h"
#include <chrono>
#include <thread>

// Vorwärtsdeklarationen
static void CreateCube(LPENTITY* mesh, LPMATERIAL material = nullptr);
static void CreateBox(LPENTITY* mesh, LPMATERIAL material, std::vector<DirectX::XMFLOAT3>& points);
static void InitializeBoxPoints(std::vector<DirectX::XMFLOAT3>& points, DirectX::XMFLOAT3 minCorner, DirectX::XMFLOAT3 maxCorner);
static void UpdateBoxWithCorners(LPENTITY mesh, unsigned int indexSurface, const DirectX::XMFLOAT3 corners[8]);

static LPSHADER g_shadertest;

void main(LPVOID)
{
    Engine::Graphics(1200, 650);

    g_shadertest = nullptr;
    Engine::CreateShader(&g_shadertest,
        L"..\\shaders\\VertexShaderRot.hlsl", "main",
        L"..\\shaders\\PixelShaderRot.hlsl",  "main",
        Engine::CreateVertexFlags(true, false, false, false, false));

    // Texturen
    LPTEXTURE texture1 = nullptr;
    Engine::LoadTexture(&texture1, L"..\\media\\engine.png");

    LPTEXTURE texture2 = nullptr;
    Engine::LoadTexture(&texture2, L"..\\media\\face.bmp");

    LPTEXTURE texture3 = nullptr;
    Engine::LoadTexture(&texture3, L"..\\media\\color3.png");

    // Materialien
    LPMATERIAL material2;
    Engine::CreateMaterial(&material2);
    Engine::MaterialTexture(material2, texture1);

    LPMATERIAL material3;
    Engine::CreateMaterial(&material3);
    Engine::MaterialTexture(material3, texture2);

    // Kamera
    LPENTITY camera;
    Engine::CreateCamera(&camera);
    Engine::RotateEntity(camera, 10.0f, 0.0f, 0.0f);
    Engine::PositionEntity(camera, 0.0f, 25.0f, -5.0f);

    // Lichter
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(light, 0.0f, 20.0f, 10.0f);
    Engine::RotateEntity(light, 90, 0, 0);
    Engine::LightColor(light, 0.4f, 0.4f, 0.5f);

    LPENTITY light2 = nullptr;
    Engine::CreateLight(&light2, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(light2, 0.0f, 10.0f, -10.0f);
    Engine::RotateEntity(light2, -45, 0, 0);
    Engine::LightColor(light2, 0.5f, 0.0f, 0.0f);

    Engine::SetDirectionalLight(light);
    Engine::SetAmbientColor(0.1f, 0.1f, 0.1f);

    // Würfel
    LPENTITY cube;
    CreateCube(&cube, material3);
    Engine::PositionEntity(cube, 0.0f, 10.0f, 10.0f);
    Engine::RotateEntity(cube, 0.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(cube, 4.0f, 4.0f, 4.0f);
    Engine::EntityCollisionMode(cube, COLLISION::BOX);

    LPENTITY cube2;
    CreateCube(&cube2, material2);
    Engine::PositionEntity(cube2, 0.0f, -10.0f, 10.0f);
    Engine::ScaleEntity(cube2, 50.0f, 0.2f, 50.0f);
    Engine::RotateEntity(cube2, 0.0f, 180.0f, 0.0f);
    Engine::EntityCollisionMode(cube2, COLLISION::BOX);

    Engine::EntityTexture(cube, texture2);
    Engine::engine->GetOM().AddMeshToMaterial(material2, dynamic_cast<Mesh*>(cube2));
    Engine::engine->GetOM().AddMeshToMaterial(material3, dynamic_cast<Mesh*>(cube));

    float speed = 100.0f;

    std::vector<DirectX::XMFLOAT3> points;
    InitializeBoxPoints(points, Engine::EntitySurface(cube, 0)->minPoint, Engine::EntitySurface(cube, 0)->maxPoint);
    std::vector<DirectX::XMFLOAT3> points2;
    InitializeBoxPoints(points2, Engine::EntitySurface(cube2, 0)->minPoint, Engine::EntitySurface(cube2, 0)->maxPoint);

    LPENTITY box;
    CreateBox(&box, material2, points);
    Engine::EntitySurface(box, 0)->test = true;

    LPENTITY box2;
    CreateBox(&box2, material2, points2);
    Engine::EntitySurface(box2, 0)->test = true;

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Core::GetDeltaTime());

        if ((GetAsyncKeyState(VK_ADD) & 0x8000))
            Engine::MoveEntity(camera, 0.0f, 5 * dt, 0.0f);

        if ((GetAsyncKeyState(VK_SUBTRACT) & 0x8000))
            Engine::MoveEntity(camera, 0.0f, -5 * dt, 0.0f);
        else
            Engine::TurnEntity(cube, speed * dt, speed * dt, 0.0f);

        if ((GetAsyncKeyState(VK_F1) & 0x8000))
            Engine::MoveEntity(camera, 0.0f, 0.0f, 2.0f * dt);

        if (Engine::EntityCollision(cube, cube2))
            Debug::LogOnce("Kollision");
        else
            Debug::LogOnce("Keine Kollision");

        DirectX::XMFLOAT3 corners[8];
        Engine::EntityOBB(cube)->GetCorners(corners);
        UpdateBoxWithCorners(box, 0, corners);

        DirectX::XMFLOAT3 corners1[8];
        Engine::EntityOBB(cube2)->GetCorners(corners1);
        UpdateBoxWithCorners(box2, 0, corners);

        Engine::LookAt(camera,
            DirectX::XMVectorGetX(cube->transform.GetPosition()),
            DirectX::XMVectorGetY(cube->transform.GetPosition()),
            DirectX::XMVectorGetZ(cube->transform.GetPosition()));

        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }
}
