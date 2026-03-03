#include "gidx.h"
#include "geometry.h" // CreateCube()

int main()
{
    bool sw = true;

    // Initialize graphics device (windowed or fullscreen depending on switch)
    sw == true ? Engine::Graphics(1200, 650)
        : Engine::Graphics(1980, 1080, false);

    // -------------------------------------------------
    // Texture loading
    // -------------------------------------------------
    LPTEXTURE texture = nullptr;
    Engine::LoadTexture(&texture, L"..\\media\\face.bmp");

    // -------------------------------------------------
    // Material setup
    // -------------------------------------------------
    LPMATERIAL material;
    Engine::CreateMaterial(&material);
    Engine::MaterialTexture(material, texture); // Assign texture to material

    // -------------------------------------------------
    // Directional light setup
    // -------------------------------------------------
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL); // Created via LightManager
    Engine::TurnEntity(light, 45, 0, 0);               // Rotate light direction
    Engine::LightColor(light, 1.0f, 1.0f, 1.0f);       // White light

    Engine::SetAmbientColor(0.0f, 0.3f, 0.3f);         // Global ambient lighting

    // -------------------------------------------------
    // Camera setup
    // -------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);                     // Managed by ObjectManager
    Engine::PositionEntity(camera, 0, 0, -5);          // Move camera backward

    // -------------------------------------------------
    // Mesh creation
    // -------------------------------------------------
    LPENTITY cube = nullptr;
    Engine::CreateMesh(&cube);                         // Create empty mesh entity

    CreateCube(&cube, material);                       // Generate cube geometry + assign material
    Engine::PositionEntity(cube, 0.0f, 0.0f, 5.0f);    // Place cube in front of camera
    Engine::RotateEntity(cube, 45.0f, 45.0f, 0.0f);    // Initial rotation
    Engine::ScaleEntity(cube, 1.0f, 1.0f, 1.0f);       // Uniform scale

    // -------------------------------------------------
    // Main loop
    // -------------------------------------------------
    while (Windows::MainLoop() &&
        !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))      // Exit on ESC
    {
        Core::BeginFrame();     // Updates delta time, FPS, frame counter

        Engine::Cls(0, 64, 128); // Clear backbuffer with background color

        Engine::UpdateWorld();   // Update all entities (transforms, logic)
        Engine::RenderWorld();   // Render entire scene
        Engine::Flip();          // Present backbuffer to screen

        Core::EndFrame();
    }

    // Shutdown handled internally on exit
    return 0;
}