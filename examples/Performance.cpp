#define NOMINMAX  // ← VOR allen includes
#include "gidx.h"
#include <chrono>
#include <vector>
#include <cmath>
#include <algorithm>

// Simple Cube Creation
void CreateSimpleCube(LPENTITY* mesh, MATERIAL* material = nullptr);

int main()
{
    // Fullscreen für bessere Performance
    Engine::Graphics(1200, 650, true);

    // Material erstellen
    LPMATERIAL material;
    Engine::CreateMaterial(&material);

    // Kamera erstellen und positionieren
    LPENTITY camera;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 60.0f, -100.0f);  // Von oben und hinten schauen
    Engine::RotateEntity(camera, 20.0f, 0.0f, 0.0f);       // Leicht nach unten geneigt

    // Light erstellen
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);    // Nutzt LightManager
    Engine::RotateEntity(light, -90, 0, 0);                // Funktioniert
    Engine::LightColor(light, 2.0f, 0.0f, 0.0f);

    LPENTITY light2 = nullptr;
    Engine::CreateLight(&light2, D3DLIGHT_DIRECTIONAL);  // Nutzt LightManager
    Engine::TurnEntity(light2, 90, 0, 0);                // Funktioniert
    Engine::LightColor(light2, 0.0f, 0.0f, 2.0f);

    LPENTITY light3 = nullptr;
    Engine::CreateLight(&light3, D3DLIGHT_DIRECTIONAL);  // Nutzt LightManager
    Engine::TurnEntity(light3, 0, 90, 0);                // Funktioniert
    Engine::LightColor(light3, 0.0f, 1.0f, 1.0f);

    LPENTITY light4 = nullptr;
    Engine::CreateLight(&light4, D3DLIGHT_DIRECTIONAL);  // Nutzt LightManager
    Engine::TurnEntity(light4, 0, -90, 0);                // Funktioniert
    Engine::LightColor(light4, 5.0f, 0.0f, 0.0f);


    Engine::SetAmbientColor(0.2f, 0.2f, 0.2f);

    // ==================== WÜRFEL ERSTELLEN ====================
    const int CUBES_X = 25;  // 50 x 50 x 4 = 10.000 Würfel
    const int CUBES_Y = 25;
    const int CUBES_Z = 25;
    const float SPACING = 2.5f;

    std::vector<LPENTITY> cubes;
    cubes.reserve(CUBES_X * CUBES_Y * CUBES_Z);

    Debug::Log("Creating ", CUBES_X * CUBES_Y * CUBES_Z, " cubes...");
    auto startCreate = std::chrono::high_resolution_clock::now();

    // Grid von Würfeln erstellen
    for (int y = 0; y < CUBES_Y; y++) {
        for (int z = 0; z < CUBES_Z; z++) {
            for (int x = 0; x < CUBES_X; x++) {
                LPENTITY cube = nullptr;
                CreateSimpleCube(&cube, material);

                // Position im Grid
                float posX = (x - CUBES_X / 2.0f) * SPACING;
                float posY = y * SPACING;
                float posZ = (z - CUBES_Z / 2.0f) * SPACING;
                Engine::PositionEntity(cube, posX, posY, posZ);

                // Zufällige Start-Rotation
                Engine::RotateEntity(cube,
                    static_cast<float>(rand() % 360),
                    static_cast<float>(rand() % 360),
                    static_cast<float>(rand() % 360));

                Engine::RotateEntity(cube,
                    0.0f,
                    0.0f,
                    0.0f);

                cubes.push_back(cube);
            }
        }
    }

    auto endCreate = std::chrono::high_resolution_clock::now();
    double createTime = std::chrono::duration<double>(endCreate - startCreate).count();
    Debug::Log("Creation took: ", createTime, " seconds");
    Debug::Log("Cubes created: ", cubes.size());

    // ==================== PERFORMANCE TRACKING ====================
    int frameCount = 0;
    double totalFrameTime = 0.0;
    double minFrameTime = 999999.0;
    double maxFrameTime = 0.0;

    auto lastFPSLog = std::chrono::high_resolution_clock::now();
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    // Rotation speeds
    float rotSpeedX = 30.0f;  // Grad pro Sekunde
    float rotSpeedY = 45.0f;
    float rotSpeedZ = 20.0f;

    Debug::Log("========================================");
    Debug::Log("Starting render loop...");
    Debug::Log("Press ESC to exit");
    Debug::Log("Press SPACE to toggle rotation");
    Debug::Log("Press UP/DOWN to move camera");
    Debug::Log("========================================");

    bool rotationEnabled = true;

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        // ==================== FRAME TIME MEASUREMENT ====================
        auto frameStart = std::chrono::high_resolution_clock::now();
        double dt = std::chrono::duration<double>(frameStart - lastFrameTime).count();
        lastFrameTime = frameStart;

        static bool spacePressed = false;
        // ==================== INPUT ====================
        //if ((GetAsyncKeyState(VK_SPACE) & 0x8000)) {
        //    if (!spacePressed) {
        //        rotationEnabled = !rotationEnabled;
        //        spacePressed = true;
        //    }
        //}
        //else {
        //    spacePressed = false;
        //}

        if ((GetAsyncKeyState(VK_SPACE) & 0x8000))
        {
            Engine::PositionEntity(light4, camera->transform.GetPosition());
            Engine::RotateEntity(light4, camera->transform.GetRotationQuaternion());
        }

        if ((GetAsyncKeyState(VK_F1) & 0x8000))
        {
            Engine::PositionEntity(light3, camera->transform.GetPosition());
            Engine::LookAt(light3, camera->transform.GetLookAt());
        }

        if ((GetAsyncKeyState(VK_UP) & 0x8000)) {
            Engine::MoveEntity(camera, 0.0f, 0.0f, 50.0f * dt);
        }
        if ((GetAsyncKeyState(VK_DOWN) & 0x8000)) {
            Engine::MoveEntity(camera, 0.0f, 0.0f, -50.0f * dt);
        }
        if ((GetAsyncKeyState(VK_RIGHT) & 0x8000)) {
            Engine::MoveEntity(camera, 50.0f * dt, 0.0f, 0.0f);
        }
        if ((GetAsyncKeyState(VK_LEFT) & 0x8000)) {
            Engine::MoveEntity(camera, -50.0f * dt, 0.0f, 0.0f);
        }

        // ==================== UPDATE CUBES ====================
        if (rotationEnabled) {
            for (auto* cube : cubes) {
                Engine::TurnEntity(cube,
                    rotSpeedX * dt,
                    rotSpeedY * dt,
                    rotSpeedZ * dt);
            }
        }

        // ==================== RENDER ====================
        Engine::Cls(20, 20, 40);  // Dunkler Hintergrund
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        // ==================== PERFORMANCE STATS ====================
        auto frameEnd = std::chrono::high_resolution_clock::now();
        double frameTime = std::chrono::duration<double>(frameEnd - frameStart).count();

        totalFrameTime += frameTime;
        minFrameTime = std::min(minFrameTime, frameTime);
        maxFrameTime = std::max(maxFrameTime, frameTime);
        frameCount++;

        // Log FPS jede Sekunde
        double timeSinceLastLog = std::chrono::duration<double>(frameEnd - lastFPSLog).count();
        if (timeSinceLastLog >= 1.0) {
            double avgFrameTime = totalFrameTime / frameCount;
            double avgFPS = 1.0 / avgFrameTime;
            double minFPS = 1.0 / maxFrameTime;
            double maxFPS = 1.0 / minFrameTime;

            //Debug::Log("========================================");
            //Debug::Log("FPS - Avg: ", static_cast<int>(avgFPS),
            //    " | Min: ", static_cast<int>(minFPS),
            //    " | Max: ", static_cast<int>(maxFPS));
            //Debug::Log("Frame Time - Avg: ", avgFrameTime * 1000.0, " ms",
            //    " | Min: ", minFrameTime * 1000.0, " ms",
            //    " | Max: ", maxFrameTime * 1000.0, " ms");
            //Debug::Log("Cubes: ", cubes.size());

            // Reset stats
            frameCount = 0;
            totalFrameTime = 0.0;
            minFrameTime = 999999.0;
            maxFrameTime = 0.0;
            lastFPSLog = frameEnd;
        }
    }

    // Shutdown the engine
    return 0;
}

// ==================== SIMPLE CUBE CREATION ====================
void CreateSimpleCube(LPENTITY* mesh, MATERIAL* material)
{
    Engine::CreateMesh(mesh, material);

    LPSURFACE surface = nullptr;
    Engine::CreateSurface(&surface, *mesh);

    // 8 Eckpunkte des Würfels
    float size = 1.0f;
    DirectX::XMFLOAT3 vertices[8] = {
        {-size, -size, -size},  // 0
        {-size, +size, -size},  // 1
        {+size, +size, -size},  // 2
        {+size, -size, -size},  // 3
        {-size, -size, +size},  // 4
        {-size, +size, +size},  // 5
        {+size, +size, +size},  // 6
        {+size, -size, +size}   // 7
    };

    // 6 Seiten, 4 Vertices pro Seite = 24 Vertices
    // Front Face (Z-)
    Engine::AddVertex(surface, vertices[0]); Engine::VertexNormal(surface, 0, 0, -1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[1]); Engine::VertexNormal(surface, 0, 0, -1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[2]); Engine::VertexNormal(surface, 0, 0, -1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[3]); Engine::VertexNormal(surface, 0, 0, -1); Engine::VertexColor(surface, 255, 255, 255);

    // Back Face (Z+)  <-- Winding gedreht
    Engine::AddVertex(surface, vertices[4]); Engine::VertexNormal(surface, 0, 0, 1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[7]); Engine::VertexNormal(surface, 0, 0, 1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[6]); Engine::VertexNormal(surface, 0, 0, 1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[5]); Engine::VertexNormal(surface, 0, 0, 1); Engine::VertexColor(surface, 255, 255, 255);

    // Left Face (X-)
    Engine::AddVertex(surface, vertices[0]); Engine::VertexNormal(surface, -1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[4]); Engine::VertexNormal(surface, -1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[5]); Engine::VertexNormal(surface, -1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[1]); Engine::VertexNormal(surface, -1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);

    // Right Face (X+)
    Engine::AddVertex(surface, vertices[3]); Engine::VertexNormal(surface, 1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[2]); Engine::VertexNormal(surface, 1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[6]); Engine::VertexNormal(surface, 1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[7]); Engine::VertexNormal(surface, 1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);

    // Bottom Face (Y-)
    Engine::AddVertex(surface, vertices[0]); Engine::VertexNormal(surface, 0, -1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[3]); Engine::VertexNormal(surface, 0, -1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[7]); Engine::VertexNormal(surface, 0, -1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[4]); Engine::VertexNormal(surface, 0, -1, 0); Engine::VertexColor(surface, 255, 255, 255);

    // Top Face (Y+)
    Engine::AddVertex(surface, vertices[1]); Engine::VertexNormal(surface, 0, 1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[5]); Engine::VertexNormal(surface, 0, 1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[6]); Engine::VertexNormal(surface, 0, 1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[2]); Engine::VertexNormal(surface, 0, 1, 0); Engine::VertexColor(surface, 255, 255, 255);

    // Indices für alle 6 Seiten (2 Triangles pro Seite)
    int offset = 0;
    for (int face = 0; face < 6; face++) {
        Engine::AddTriangle(surface, offset + 0, offset + 1, offset + 2);
        Engine::AddTriangle(surface, offset + 0, offset + 2, offset + 3);
        offset += 4;
    }

    Engine::FillBuffer(surface);
}