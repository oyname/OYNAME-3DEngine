// MultiSurfaceExample.cpp
//
// Multi-Surface / Multi-Material Demo
// ------------------------------------
// This example demonstrates how a single mesh can contain multiple
// surfaces, each using a different material.
//
// The cube consists of 6 separate surfaces (one per face),
// distributed across 3 materials.
// The floor plate shares one of the cube materials to illustrate
// material-based batching behavior.

#include "gidx.h"
#include "geometry.h"

LPENTITY g_cubeMesh = nullptr;
LPENTITY g_plateMesh = nullptr;
LPENTITY g_camera = nullptr;
LPENTITY g_directionalLight = nullptr;

int main()
{
    // -------------------------------------------------
    // Engine Initialization
    // -------------------------------------------------
    Engine::Graphics(1024, 768);
    Engine::SetVSync(1);

    // -------------------------------------------------
    // Texture Loading
    // -------------------------------------------------
    LPTEXTURE texBrick = nullptr;
    LPTEXTURE texFace = nullptr;
    LPTEXTURE texBricks = nullptr;

    Engine::LoadTexture(&texBrick, L"..\\media\\dx.bmp");
    Engine::LoadTexture(&texFace, L"..\\media\\face.bmp");
    Engine::LoadTexture(&texBricks, L"..\\media\\color3.png");

    // -------------------------------------------------
    // Material Setup
    // -------------------------------------------------
    // Front face material
    LPMATERIAL matFront = nullptr;
    Engine::CreateMaterial(&matFront);
    Engine::MaterialTexture(matFront, texBrick);

    // Side faces material
    LPMATERIAL matSide = nullptr;
    Engine::CreateMaterial(&matSide);
    Engine::MaterialTexture(matSide, texFace);

    // Top & bottom material
    LPMATERIAL matTopBot = nullptr;
    Engine::CreateMaterial(&matTopBot);
    Engine::MaterialTexture(matTopBot, texBricks);

    // -------------------------------------------------
    // Multi-Surface Cube Creation
    // -------------------------------------------------
    // Each face is its own surface but belongs to the same mesh entity.
    CreateMultiMaterialCube(&g_cubeMesh, matFront, matSide, matTopBot);

    Engine::PositionEntity(g_cubeMesh, 0.0f, 40.0f, 0.0f);
    Engine::ScaleEntity(g_cubeMesh, 8.0f, 8.0f, 8.0f);

    // -------------------------------------------------
    // Floor Plate (shares matTopBot)
    // -------------------------------------------------
    // Demonstrates that different meshes using the same material
    // can end up in the same render queue bucket.
    Engine::CreateMesh(&g_plateMesh);
    CreatePlate(&g_plateMesh);
    Engine::SurfaceMaterial(Engine::GetSurface(g_plateMesh), matTopBot);

    Engine::PositionEntity(g_plateMesh, 0.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(g_plateMesh, 100.0f, 1.0f, 100.0f);

    // -------------------------------------------------
    // Camera Setup
    // -------------------------------------------------
    Engine::CreateCamera(&g_camera);
    Engine::PositionEntity(g_camera, 0.0f, 30.0f, -90.0f);
    Engine::RotateEntity(g_camera, 10.0f, 0.0f, 0.0f);

    // -------------------------------------------------
    // Directional Light Setup
    // -------------------------------------------------
    Engine::CreateLight(&g_directionalLight, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(g_directionalLight, 20.0f, 60.0f, -20.0f);
    Engine::RotateEntity(g_directionalLight, 45.0f, -45.0f, 0.0f);
    Engine::LightColor(g_directionalLight, 0.8f, 0.8f, 0.8f);

    // Enable shadow casting
    Engine::SetDirectionalLight(g_directionalLight);

    // Global ambient contribution
    Engine::SetAmbientColor(0.2f, 0.2f, 0.2f);

    // -------------------------------------------------
    // Main Loop
    // -------------------------------------------------
    const float rotationSpeed = 80.0f;

    Engine::DebugPrintScene();

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();

        const float dt = static_cast<float>(Timer::GetDeltaTime());

        // Rotate cube continuously to showcase all surfaces/materials
        Engine::TurnEntity(
            g_cubeMesh,
            rotationSpeed * dt,
            rotationSpeed * dt * 0.7f,
            0.0f
        );

        Engine::Cls(0, 64, 128);

        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    return 0;
}