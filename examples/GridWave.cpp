#define NOMINMAX  
#include "gidx.h"
#include <chrono>
#include <vector>
#include <cmath>
#include <algorithm>

struct GridBase { float x, z; };
static inline int VIdx(int x, int z, int vertsX) { return z * vertsX + x; }

// Creates a flat grid (X/Z plane). Vertices are written by explicit index (0..N-1)
// so we can update them each frame using Engine::AddVertex(index, surface, pos).
static void CreateWaveGrid(LPENTITY* mesh,
    LPMATERIAL material,
    int cellsX, int cellsZ,
    float sizeX, float sizeZ,
    std::vector<GridBase>& outBase,
    unsigned int& outSurfaceIndex)
{
    Engine::CreateMesh(mesh, material);

    LPSURFACE surface = nullptr;
    Engine::CreateSurface(&surface, *mesh);

    outSurfaceIndex = 0;

    const int vertsX = cellsX + 1;
    const int vertsZ = cellsZ + 1;
    const int vertCount = vertsX * vertsZ;

    outBase.clear();
    outBase.resize(vertCount);

    const float halfX = sizeX * 0.5f;
    const float halfZ = sizeZ * 0.5f;

    int vid = 0;
    for (int z = 0; z < vertsZ; ++z)
    {
        float tz = (float)z / (float)cellsZ;  // 0..1
        float wz = -halfZ + tz * sizeZ;

        for (int x = 0; x < vertsX; ++x)
        {
            float tx = (float)x / (float)cellsX;  // 0..1
            float wx = -halfX + tx * sizeX;

            outBase[vid] = { wx, wz };

            // Position
            Engine::AddVertex(vid, surface, DirectX::XMFLOAT3(wx, 0.0f, wz));

            // Normal
            Engine::VertexNormal(surface, 0.0f, 1.0f, 0.0f);

            // Color
            Engine::VertexColor(surface, 255, 255, 255);

            // tx und tz gehen bereits von 0 bis 1
            Engine::VertexTexCoord(surface, tx, tz);

            //Alternative: Wenn du die Textur mehrfach wiederholen willst:
            //float repeatU = 1.0f;  // Textur 5x in X-Richtung
            //float repeatV = 1.0f;  // Textur 5x in Z-Richtung
            //Engine::VertexTexCoord(surface, tx * repeatU, tz * repeatV);

            ++vid;
        }
    }

    // Triangles (2 per cell)
    for (int z = 0; z < cellsZ; ++z)
    {
        for (int x = 0; x < cellsX; ++x)
        {
            int i0 = VIdx(x, z, vertsX);
            int i1 = VIdx(x + 1, z, vertsX);
            int i2 = VIdx(x, z + 1, vertsX);
            int i3 = VIdx(x + 1, z + 1, vertsX);

            // If culling is wrong, swap winding here.
            Engine::AddTriangle(surface, i0, i2, i1);
            Engine::AddTriangle(surface, i1, i2, i3);
        }
    }

    // Build initial buffers once
    Engine::FillBuffer(surface);
}

static void UpdateGridRadialWave(LPENTITY mesh,
    unsigned int surfaceIndex,
    const std::vector<GridBase>& base,
    float tSeconds,
    float amplitude,
    float wavelength,
    float speed,
    int vertsX, int vertsZ)
{
    auto surface = Engine::GetSurface(mesh);

    const float k = 6.28318530718f / wavelength;

    // 1. Positions & Normals parallel berechnen
    for (int z = 0; z < vertsZ; ++z)
    {
        for (int x = 0; x < vertsX; ++x)
        {
            int idx = VIdx(x, z, vertsX);

            const float px = base[idx].x;
            const float pz = base[idx].z;
            const float r = std::sqrt(px * px + pz * pz);
            const float y = amplitude * std::sin(k * r - speed * tSeconds);

            // ==================== ANALYTISCHE NORMALE ====================
            // Statt Nachbarn zu samplen, berechnen wir die Ableitung direkt!
            // Die Welle ist: y = A * sin(k*r - w*t)
            // Ableitung: dy/dr = A * k * cos(k*r - w*t)

            if (r > 0.0001f) // Zentrum vermeiden (Division durch 0)
            {
                const float dydr = amplitude * k * std::cos(k * r - speed * tSeconds);

                // Gradient in X und Z
                const float dydx = dydr * (px / r);  // dy/dx = (dy/dr) * (dr/dx)
                const float dydz = dydr * (pz / r);  // dy/dz = (dy/dr) * (dr/dz)

                // Normale = normalize(-dydx, 1, -dydz)
                float nx = -dydx;
                float ny = 1.0f;
                float nz = -dydz;

                float len = std::sqrt(nx * nx + ny * ny + nz * nz);
                nx /= len;
                ny /= len;
                nz /= len;

                Engine::AddVertex(idx, surface, DirectX::XMFLOAT3(px, y, pz));
                Engine::VertexNormal(surface, idx, nx, ny, nz);
            }
            else
            {
                // Im Zentrum: Normale zeigt nach oben
                Engine::AddVertex(idx, surface, DirectX::XMFLOAT3(px, y, pz));
                Engine::VertexNormal(surface, idx, 0.0f, 1.0f, 0.0f);
            }
        }
    }

    Engine::UpdateVertexBuffer(surface);
}

int main()
{
    Engine::Graphics(1280, 900, true);

    // Material
    LPMATERIAL material;
    Engine::CreateMaterial(&material);
    LPTEXTURE texture = nullptr;
    Engine::LoadTexture(&texture, L"F:\\VisualStudio\\media\\dx.bmp");
    Engine::MaterialTexture(material, texture);

    // Camera
    LPENTITY camera;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 80.0f, -140.0f);
    Engine::RotateEntity(camera, 25.0f, 0.0f, 0.0f);

    // Light
    LPENTITY light;
    Engine::CreateLight(&light, D3DLIGHTTYPE::D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(light, 0.0f, 50.0f, 0.0f);
    Engine::RotateEntity(light, 45.0f, 0.0f, 0.0f);
    Engine::LightColor(light, 1.0f, 0.8f, 0.6f);  // Warmes Diffuse
    Engine::SetAmbientColor(1.0f, 1.0f, 1.0f);  // Globales Ambient (nicht mehr pro Licht!)

    // ==================== GRID SETUP ====================
    const int CELLS_X = 250;      // resolution (more => heavier)
    const int CELLS_Z = 250;
    const float SIZE_X = 200.0f;  // world size
    const float SIZE_Z = 200.0f;

    LPENTITY grid = nullptr;
    unsigned int gridSurfaceIndex = 0;
    std::vector<GridBase> gridBase;

    auto startCreate = std::chrono::high_resolution_clock::now();

    CreateWaveGrid(&grid, material, CELLS_X, CELLS_Z, SIZE_X, SIZE_Z, gridBase, gridSurfaceIndex);

    const int vertsX = CELLS_X + 1;
    const int vertsZ = CELLS_Z + 1;

    auto endCreate = std::chrono::high_resolution_clock::now();
    double createTime = std::chrono::duration<double>(endCreate - startCreate).count();

    int frameCount = 0;
    double totalFrameTime = 0.0;
    double minFrameTime = 999999.0;
    double maxFrameTime = 0.0;

    auto lastFPSLog = std::chrono::high_resolution_clock::now();
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    Debug::Log("========================================");
    Debug::Log("Starting render loop...");
    Debug::Log("Press ESC to exit");
    Debug::Log("Press SPACE to toggle wave animation");
    Debug::Log("Press UP/DOWN to move camera");
    Debug::Log("========================================");

    // Wave parameters
    float amplitude = 3.0f;
    float wavelength = 12.0f;
    float speed = 4.0f;

    bool waveEnabled = true;

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();

        const float dt = static_cast<float>(Core::GetDeltaTime());

        static bool spacePressed = false;
        // ==================== INPUT ====================
        if ((GetAsyncKeyState(VK_SPACE) & 0x8000)) {
            if (!spacePressed) {
                waveEnabled = !waveEnabled;
                spacePressed = true;
            }
        }
        else {
            spacePressed = false;
        }

        if ((GetAsyncKeyState(VK_UP) & 0x8000)) { Engine::MoveEntity(camera, 0.0f, 0.0f, 50.0f * dt); }
        if ((GetAsyncKeyState(VK_DOWN) & 0x8000)) { Engine::MoveEntity(camera, 0.0f, 0.0f, -50.0f * dt); }
        if ((GetAsyncKeyState(VK_RIGHT) & 0x8000)) { Engine::MoveEntity(camera, 50.0f * dt, 0.0f, 0.0f * dt); }
        if ((GetAsyncKeyState(VK_LEFT) & 0x8000)) { Engine::MoveEntity(camera, -50.0f * dt, 0.0f, 0.0f * dt); }

        Engine::LookAt(camera, DirectX::XMVectorGetX(grid->transform.GetPosition()),
                               DirectX::XMVectorGetY(grid->transform.GetPosition()),
                               DirectX::XMVectorGetZ(grid->transform.GetPosition()));

        // Update grid
        if (waveEnabled)
        {
            static double tAccum = 0.0;
            tAccum += dt;
            UpdateGridRadialWave(grid, gridSurfaceIndex, gridBase,
                (float)tAccum, amplitude, wavelength, speed,
                vertsX, vertsZ);
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