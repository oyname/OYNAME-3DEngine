#include "gidx.h"
#include "geometry.h" // CreateCube()

/*
    Layer + visibility demo
    -----------------------
    This sample shows how to control what gets rendered using:
      - Camera cull masks (layer-based rendering)
      - Per-entity visibility (ShowEntity)
      - Per-entity activity (EntityActive)

    Scene:
      - A floor plate (DEFAULT | REFLECTION layer)
      - Cube 1 on DEFAULT layer (rotates continuously)
      - Cube 2 on FX layer

    Controls:
      Arrow keys : Move the red point light
      [1]        : Toggle DEFAULT layer in the camera cull mask (cube1 + plate on/off)
      [2]        : Toggle FX layer in the camera cull mask (cube2 on/off)
      [3]        : Toggle visibility of cube1 (still updates/rotates)
      [4]        : Toggle active state of cube2 (freezes update + stops rendering)
      ESC        : Exit
*/

static LPENTITY g_plateMesh = nullptr;
static LPENTITY g_cubeMesh = nullptr;
static LPENTITY g_cubeMesh2 = nullptr;
static LPENTITY g_camera = nullptr;
static LPENTITY g_redLight = nullptr;
static LPENTITY g_blueLight = nullptr;
static LPENTITY g_directionalLight = nullptr;

static inline bool KeyDown(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

int main()
{
    Engine::Graphics(1024, 768);
    Engine::SetVSync(1);

    // -------------------------------------------------
    // Textures
    // -------------------------------------------------
    LPTEXTURE texCube1 = nullptr;
    LPTEXTURE texFloor = nullptr;
    LPTEXTURE texCube2 = nullptr;

    Engine::LoadTexture(&texCube1, L"..\\media\\color1.png");
    Engine::LoadTexture(&texFloor, L"..\\media\\bricks.bmp");
    Engine::LoadTexture(&texCube2, L"..\\media\\color3.png");

    // -------------------------------------------------
    // Camera (sees all layers by default)
    // -------------------------------------------------
    Engine::CreateCamera(&g_camera);
    Engine::PositionEntity(g_camera, 0.0f, 30.0f, -90.0f);
    Engine::RotateEntity(g_camera, 10.0f, 0.0f, 0.0f);
    Engine::CameraCullMask(g_camera, LAYER_ALL);

    // -------------------------------------------------
    // Materials
    // -------------------------------------------------
    LPMATERIAL matCube1 = nullptr;
    LPMATERIAL matFloor = nullptr;
    LPMATERIAL matCube2 = nullptr;

    Engine::CreateMaterial(&matCube1);
    Engine::MaterialTexture(matCube1, texCube1);

    Engine::CreateMaterial(&matFloor);
    Engine::MaterialTexture(matFloor, texFloor);

    Engine::CreateMaterial(&matCube2);
    Engine::MaterialTexture(matCube2, texCube2);

    // -------------------------------------------------
    // Geometry
    // -------------------------------------------------
    // Cube 1 (DEFAULT layer)
    CreateCube(&g_cubeMesh, matCube1);
    Engine::PositionEntity(g_cubeMesh, 0.0f, 35.0f, 0.0f);
    Engine::ScaleEntity(g_cubeMesh, 8.0f, 8.0f, 8.0f);
    Engine::EntityLayer(g_cubeMesh, LAYER_DEFAULT);

    // Cube 2 (FX layer)
    CreateCube(&g_cubeMesh2, matCube2);
    Engine::PositionEntity(g_cubeMesh2, 0.0f, 6.0f, 0.0f);
    Engine::ScaleEntity(g_cubeMesh2, 5.0f, 5.0f, 5.0f);
    Engine::EntityLayer(g_cubeMesh2, LAYER_FX);

    // Floor plate (DEFAULT | REFLECTION layers)
    CreateCube(&g_plateMesh, matFloor);
    Engine::PositionEntity(g_plateMesh, 0.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(g_plateMesh, 100.0f, 1.0f, 100.0f);
    Engine::EntityLayer(g_plateMesh, LAYER_DEFAULT | LAYER_REFLECTION);

    // -------------------------------------------------
    // Lights
    // -------------------------------------------------
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

    // Rotation speed (deg/sec)
    const float rotSpeed = 100.0f;

    // Debounce state for toggles
    bool key1Last = false, key2Last = false, key3Last = false, key4Last = false;

    while (Windows::MainLoop() && !KeyDown(VK_ESCAPE))
    {
        Core::BeginFrame();
        const float dt = (float)Core::GetDeltaTime();

        // -------------------------------------------------
        // Light controls (move red point light)
        // -------------------------------------------------
        const float lightMove = 50.0f;
        if (KeyDown(VK_UP))    Engine::MoveEntity(g_redLight, 0.0f, 0.0f, lightMove * dt);
        if (KeyDown(VK_DOWN))  Engine::MoveEntity(g_redLight, 0.0f, 0.0f, -lightMove * dt);
        if (KeyDown(VK_RIGHT)) Engine::MoveEntity(g_redLight, lightMove * dt, 0.0f, 0.0f);
        if (KeyDown(VK_LEFT))  Engine::MoveEntity(g_redLight, -lightMove * dt, 0.0f, 0.0f);

        // -------------------------------------------------
        // Layer toggles (camera cull mask)
        // -------------------------------------------------
        // [1] Toggle DEFAULT layer (cube1 + plate)
        const bool key1 = KeyDown('1');
        if (key1 && !key1Last)
        {
            uint32_t mask = Engine::CameraCullMask(g_camera);
            mask ^= LAYER_DEFAULT;
            Engine::CameraCullMask(g_camera, mask);
        }
        key1Last = key1;

        // [2] Toggle FX layer (cube2)
        const bool key2 = KeyDown('2');
        if (key2 && !key2Last)
        {
            uint32_t mask = Engine::CameraCullMask(g_camera);
            mask ^= LAYER_FX;
            Engine::CameraCullMask(g_camera, mask);
        }
        key2Last = key2;

        // -------------------------------------------------
        // Entity state toggles
        // -------------------------------------------------
        // [3] Toggle visibility of cube1 (still updates)
        const bool key3 = KeyDown('3');
        if (key3 && !key3Last)
        {
            const bool nowVisible = !Engine::EntityVisible(g_cubeMesh);
            Engine::ShowEntity(g_cubeMesh, nowVisible);
        }
        key3Last = key3;

        // [4] Toggle active state of cube2 (freezes update + stops rendering)
        const bool key4 = KeyDown('4');
        if (key4 && !key4Last)
        {
            const bool nowActive = !Engine::EntityActive(g_cubeMesh2);
            Engine::EntityActive(g_cubeMesh2, nowActive);
        }
        key4Last = key4;

        // -------------------------------------------------
        // Update & Render
        // -------------------------------------------------
        Engine::TurnEntity(g_cubeMesh, rotSpeed * dt, rotSpeed * dt, 0.0f);

        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    return 0;
}