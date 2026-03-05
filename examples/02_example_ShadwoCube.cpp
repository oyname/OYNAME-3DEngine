#include "gidx.h"
#include "geometry.h"

// Simple demo scene globals (kept as pointers because the engine API uses them)
static LPENTITY  g_plateMesh = nullptr;
static LPENTITY  g_cubeMesh = nullptr;
static LPENTITY  g_camera = nullptr;
static LPENTITY  g_redLight = nullptr;
static LPENTITY  g_blueLight = nullptr;
static LPENTITY  g_directionalLight = nullptr;

// Must match main.h: int main(LPVOID hwnd)
int main(LPVOID hwnd)
{
    // Graphics mode (engine core is already initialized by WinMain)
    Engine::Graphics(1024, 768);

    // -------------------------------------------------
    // Textures
    // -------------------------------------------------
    LPTEXTURE texCube = nullptr;
    LPTEXTURE texFloor = nullptr;

    Engine::LoadTexture(&texCube, L"..\\media\\dx.bmp");
    Engine::LoadTexture(&texFloor, L"..\\media\\bricks.bmp");

    // -------------------------------------------------
    // Camera
    // -------------------------------------------------
    Engine::CreateCamera(&g_camera);
    Engine::PositionEntity(g_camera, 0.0f, 40.0f, -90.0f);
    Engine::RotateEntity(g_camera, 25.0f, 0.0f, 0.0f);

    // -------------------------------------------------
    // Materials
    // -------------------------------------------------
    LPMATERIAL matCube = nullptr;
    LPMATERIAL matFloor = nullptr;

    Engine::CreateMaterial(&matCube);
    Engine::MaterialTexture(matCube, texCube);

    Engine::CreateMaterial(&matFloor);
    Engine::MaterialTexture(matFloor, texFloor);

    // -------------------------------------------------
    // Geometry (cube + floor plate)
    // -------------------------------------------------
    Engine::CreateMesh(&g_cubeMesh);
    CreateCube(&g_cubeMesh, matCube);
    Engine::PositionEntity(g_cubeMesh, 0.0f, 25.0f, 0.0f);
    Engine::ScaleEntity(g_cubeMesh, 5.0f, 5.0f, 5.0f);

    Engine::CreateMesh(&g_plateMesh);
    CreateCube(&g_plateMesh, matFloor);
    Engine::PositionEntity(g_plateMesh, 0.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(g_plateMesh, 50.0f, 0.5f, 50.0f);

    // -------------------------------------------------
    // Lights
    // -------------------------------------------------
    // Directional light = shadow caster (important: SetDirectionalLight enables shadow map usage)
    Engine::CreateLight(&g_directionalLight, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(g_directionalLight, 0.0f, 150.0f, 0.0f);
    Engine::RotateEntity(g_directionalLight, 90.0f, 0.0f, 0.0f);
    Engine::LightColor(g_directionalLight, 1.0f, 1.0f, 1.0f);
    Engine::SetDirectionalLight(g_directionalLight);

    // Two small point lights for colored fill (optional)
    Engine::CreateLight(&g_redLight, D3DLIGHT_POINT);
    Engine::PositionEntity(g_redLight, 20.0f, 15.0f, 0.0f);
    Engine::LightColor(g_redLight, 1.0f, 0.3f, 0.3f);

    Engine::CreateLight(&g_blueLight, D3DLIGHT_POINT);
    Engine::PositionEntity(g_blueLight, -20.0f, 15.0f, 0.0f);
    Engine::LightColor(g_blueLight, 0.3f, 0.3f, 1.0f);

    Engine::SetVSync(1);

    // -------------------------------------------------
    // Main loop
    // -------------------------------------------------
    const float moveSpeed = 50.0f;   // units/sec
    const float rotSpeed = 100.0f;  // deg/sec

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Timer::GetDeltaTime());

        // Basic input: move cube on X/Z plane
        if (GetAsyncKeyState(VK_UP) & 0x8000) Engine::MoveEntity(g_cubeMesh, 0.0f, 0.0f, moveSpeed * dt);
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) Engine::MoveEntity(g_cubeMesh, 0.0f, 0.0f, -moveSpeed * dt);
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Engine::MoveEntity(g_cubeMesh, moveSpeed * dt, 0.0f, 0.0f);
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) Engine::MoveEntity(g_cubeMesh, -moveSpeed * dt, 0.0f, 0.0f);

        // Continuous rotation (makes the shadow movement obvious)
        Engine::TurnEntity(g_cubeMesh, rotSpeed * dt, rotSpeed * dt, 0.0f);

        // Frame rendering
        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    // WinMain/Core shutdown happens outside (do NOT call Core::Shutdown() here)
    Debug::Log("ShadowCube demo finished. Last FPS: ", Core::GetFPS());
    return 0;
}
