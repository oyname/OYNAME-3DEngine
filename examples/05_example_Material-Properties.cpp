#include "gidx.h"
#include "geometry.h" // CreateCube()


#include "gidx.h"
#include "geometry.h" // CreateCube()

/*
    PBR Material Demo
    -----------------
    This sample demonstrates:

    - Creating a camera and a directional light
    - Setting up a PBR material (Albedo + Normal + ORM)
    - Applying metallic, roughness and occlusion parameters
    - Using emissive color and UV tiling
    - Rendering a rotating cube with lighting and shadow support

    The cube rotates continuously so you can observe:
    - Specular highlights (metallic/roughness)
    - Normal map surface detail
    - Emissive contribution
*/

int main()
{
    // -------------------------------------------------
    // Engine / Window Initialization
    // -------------------------------------------------
    Engine::Graphics(1280, 720);


    // -------------------------------------------------
    // Camera Setup
    // -------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);

    // Position camera slightly back on Z axis
    Engine::PositionEntity(camera, 0.0f, 0.0f, -10.0f);

    // Look toward the scene center
    Engine::LookAt(camera, 0.0f, 0.0f, 5.0f);

    Engine::SetCamera(camera);

    // -------------------------------------------------
    // Directional Light Setup
    // -------------------------------------------------
    LPENTITY directionalLight = nullptr;
    Engine::CreateLight(&directionalLight, D3DLIGHT_DIRECTIONAL);

    // Place light above and offset
    Engine::PositionEntity(directionalLight, 20.0f, 60.0f, -20.0f);

    // Aim light toward scene
    Engine::LookAt(directionalLight, 0.0f, 0.0f, 5.0f);

    Engine::LightColor(directionalLight, 0.8f, 0.8f, 0.8f);

    // Enable shadow casting from this directional light
    Engine::SetDirectionalLight(directionalLight);

    // Global ambient light contribution
    Engine::SetAmbientColor(0.2f, 0.2f, 0.2f);

    Engine::SetVSync(1);

    // -------------------------------------------------
    // Texture Loading
    // -------------------------------------------------
    LPTEXTURE albedoTex = nullptr;
    LPTEXTURE normalTex = nullptr;
    LPTEXTURE ormTex = nullptr;

    Engine::LoadTexture(&albedoTex, L"..\\media\\albedo.png");
    Engine::LoadTexture(&normalTex, L"..\\media\\normal.png");
    Engine::LoadTexture(&ormTex, L"..\\media\\orm.png");

    // -------------------------------------------------
    // Material Creation (PBR Setup)
    // -------------------------------------------------
    LPMATERIAL matCube = nullptr;
    Engine::CreateMaterial(&matCube);

    // Texture slot convention:
    // t0 = Albedo
    // t1 = Normal
    // t2 = ORM (Occlusion, Roughness, Metallic)

    Engine::MaterialTexture(matCube, albedoTex, 0);
    Engine::MaterialTexture(matCube, normalTex, 1);
    Engine::MaterialTexture(matCube, ormTex, 2);

    // Base color multiplier
    Engine::MaterialColor(matCube, 1.0f, 1.0f, 1.0f, 1.0f);

    // PBR scalar parameters
    Engine::MaterialMetallic(matCube, 1.0f);
    Engine::MaterialRoughness(matCube, 0.25f);
    Engine::MaterialNormalScale(matCube, 1.0f);
    Engine::MaterialOcclusionStrength(matCube, 1.0f);

    // Emissive contribution (adds glow independent of lighting)
    Engine::MaterialEmissiveColor(matCube, 1.0f, 0.3f, 0.0f, 1.5f);

    // UV tiling (repeat texture 2x in both directions)
    Engine::MaterialUVTilingOffset(matCube, 2.0f, 2.0f, 0.0f, 0.0f);

    // -------------------------------------------------
    // Mesh Creation
    // -------------------------------------------------
    LPENTITY cube = nullptr;
    CreateCube(&cube, matCube);

    // Place cube in front of the camera
    Engine::PositionEntity(cube, 0.0f, 0.0f, 5.0f);

    Engine::DebugPrintScene();

    // -------------------------------------------------
    // Main Render Loop
    // -------------------------------------------------
    while (Windows::MainLoop())
    {
        Core::BeginFrame();

        // Frame delta time (seconds)
        float dt = (float)Timer::GetDeltaTime();

        // Continuous rotation around Y axis
        // This makes lighting and reflections clearly visible
        Engine::TurnEntity(cube, 0.0f, 45.0f * dt, 0.0f);

        // Clear screen (dark blue background)
        Engine::Cls(0, 64, 128);

        // Update transforms and internal systems
        Engine::UpdateWorld();

        // Render entire scene
        Engine::RenderWorld();

        // Present back buffer
        Engine::Flip();

        Core::EndFrame();
    }

    return 0;
}