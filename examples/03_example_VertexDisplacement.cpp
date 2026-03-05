#define NOMINMAX
#include "gidx.h"
#include "geometry.h"

#include <vector>
#include <cmath>

// Stores the original (static) X/Z position for each vertex.
// We keep this so we can rebuild the animated Y every frame.
struct GridBase
{
    float x;
    float z;
};

static inline int VIdx(int x, int z, int vertsX) { return z * vertsX + x; }

// -------------------------------------------------
// Create a flat grid mesh (X/Z plane) with indexed vertices.
// Important: we write vertices by explicit index so we can later update them
// using Engine::AddVertex(index, surface, pos) every frame.
// -------------------------------------------------
static void CreateWaveGrid(
    LPENTITY* mesh,
    LPMATERIAL material,
    int cellsX, int cellsZ,
    float sizeX, float sizeZ,
    std::vector<GridBase>& outBase)
{
    Engine::CreateMesh(mesh, material);

    LPSURFACE surface = nullptr;
    Engine::CreateSurface(&surface, *mesh);

    const int vertsX = cellsX + 1;
    const int vertsZ = cellsZ + 1;
    const int vertCount = vertsX * vertsZ;

    outBase.assign(vertCount, { 0.0f, 0.0f });

    const float halfX = sizeX * 0.5f;
    const float halfZ = sizeZ * 0.5f;

    // Build vertices
    int vid = 0;
    for (int z = 0; z < vertsZ; ++z)
    {
        const float tz = (float)z / (float)cellsZ;   // 0..1
        const float wz = -halfZ + tz * sizeZ;

        for (int x = 0; x < vertsX; ++x)
        {
            const float tx = (float)x / (float)cellsX; // 0..1
            const float wx = -halfX + tx * sizeX;

            outBase[vid] = { wx, wz };

            // Vertex position (flat)
            Engine::AddVertex(vid, surface, DirectX::XMFLOAT3(wx, 0.0f, wz));

            // Initial normal (up). We will overwrite normals during animation.
            Engine::VertexNormal(surface, 0.0f, 1.0f, 0.0f);

            // White vertex color
            Engine::VertexColor(surface, 255, 255, 255);

            // UV in 0..1 (one texture tile across the grid)
            Engine::VertexTexCoord(surface, tx, tz);

            ++vid;
        }
    }

    // Build triangles (2 per cell)
    for (int z = 0; z < cellsZ; ++z)
    {
        for (int x = 0; x < cellsX; ++x)
        {
            const int i0 = VIdx(x, z, vertsX);
            const int i1 = VIdx(x + 1, z, vertsX);
            const int i2 = VIdx(x, z + 1, vertsX);
            const int i3 = VIdx(x + 1, z + 1, vertsX);

            // Winding order: swap if backface culling looks wrong.
            Engine::AddTriangle(surface, i0, i2, i1);
            Engine::AddTriangle(surface, i1, i2, i3);
        }
    }

    // Build GPU buffers once. After that we only update the vertex buffer.
    Engine::FillBuffer(surface);
}

// -------------------------------------------------
// Update the grid using a radial sine wave and an analytic normal.
// Wave: y = A * sin(k*r - w*t)
// where r = sqrt(x^2 + z^2), k = 2*pi/wavelength
//
// We update:
//  - vertex position (only Y changes)
//  - per-vertex normal (so lighting reacts correctly)
//  - then push the updated vertex buffer to the GPU
// -------------------------------------------------
static void UpdateGridRadialWave(
    LPENTITY mesh,
    const std::vector<GridBase>& base,
    float tSeconds,
    float amplitude,
    float wavelength,
    float speed,
    int vertsX, int vertsZ)
{
    LPSURFACE surface = Engine::GetSurface(mesh);

    const float k = 6.28318530718f / wavelength; // 2*pi / lambda

    for (int z = 0; z < vertsZ; ++z)
    {
        for (int x = 0; x < vertsX; ++x)
        {
            const int idx = VIdx(x, z, vertsX);

            const float px = base[idx].x;
            const float pz = base[idx].z;

            const float r = std::sqrt(px * px + pz * pz);
            const float phase = k * r - speed * tSeconds;

            const float y = amplitude * std::sin(phase);

            // dy/dr = A*k*cos(phase)
            // Convert to gradient in X/Z and build normal: normalize(-dydx, 1, -dydz)
            float nx = 0.0f, ny = 1.0f, nz = 0.0f;

            if (r > 0.0001f)
            {
                const float dydr = amplitude * k * std::cos(phase);
                const float dydx = dydr * (px / r);
                const float dydz = dydr * (pz / r);

                nx = -dydx;
                nz = -dydz;

                const float len = std::sqrt(nx * nx + ny * ny + nz * nz);
                nx /= len; ny /= len; nz /= len;
            }

            // Update vertex position by index (key point of this demo)
            Engine::AddVertex(idx, surface, DirectX::XMFLOAT3(px, y, pz));

            // Update normal by vertex index
            Engine::VertexNormal(surface, idx, nx, ny, nz);
        }
    }

    // Upload updated vertices to GPU
    Engine::UpdateVertexBuffer(surface);
}

int main()
{
    Engine::Graphics(1280, 900, true);

    // -------------------------------------------------
    // Material (simple textured grid)
    // -------------------------------------------------
    LPMATERIAL material = nullptr;
    Engine::CreateMaterial(&material);

    LPTEXTURE texture = nullptr;
    Engine::LoadTexture(&texture, L"..\\media\\dx.bmp");
    Engine::MaterialTexture(material, texture);

    // -------------------------------------------------
    // Camera
    // -------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 80.0f, -140.0f);
    Engine::RotateEntity(camera, 25.0f, 0.0f, 0.0f);

    // -------------------------------------------------
    // Light (basic directional)
    // -------------------------------------------------
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::RotateEntity(light, 45.0f, 0.0f, 0.0f);
    Engine::LightColor(light, 1.0f, 0.8f, 0.6f);

    // Global ambient (kept moderate so the wave shading is visible)
    Engine::SetAmbientColor(0.25f, 0.25f, 0.25f);

    // -------------------------------------------------
    // Grid setup (dynamic vertex update demo)
    // -------------------------------------------------
    const int   CELLS_X = 250;
    const int   CELLS_Z = 250;
    const float SIZE_X = 200.0f;
    const float SIZE_Z = 200.0f;

    const int vertsX = CELLS_X + 1;
    const int vertsZ = CELLS_Z + 1;

    LPENTITY grid = nullptr;
    std::vector<GridBase> gridBase;

    CreateWaveGrid(&grid, material, CELLS_X, CELLS_Z, SIZE_X, SIZE_Z, gridBase);

    // Wave parameters
    float amplitude = 3.0f;
    float wavelength = 12.0f;
    float speed = 4.0f;

    bool waveEnabled = true;
    double tAccum = 0.0;

    // -------------------------------------------------
    // Main loop
    // -------------------------------------------------
    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();
        const float dt = (float)Timer::GetDeltaTime();

        // Toggle wave animation with SPACE (edge-triggered)
        static bool spaceWasDown = false;
        const bool spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
        if (spaceDown && !spaceWasDown) waveEnabled = !waveEnabled;
        spaceWasDown = spaceDown;

        // Basic camera movement (arrow keys)
        const float camSpeed = 80.0f;
        if (GetAsyncKeyState(VK_UP) & 0x8000) Engine::MoveEntity(camera, 0.0f, 0.0f, camSpeed * dt);
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) Engine::MoveEntity(camera, 0.0f, 0.0f, -camSpeed * dt);
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Engine::MoveEntity(camera, camSpeed * dt, 0.0f, 0.0f);
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) Engine::MoveEntity(camera, -camSpeed * dt, 0.0f, 0.0f);

        // Keep camera looking at world origin (center of the grid)
        Engine::LookAt(camera, 0.0f, 0.0f, 0.0f);

        // Animate the wave by modifying vertex positions/normals each frame
        if (waveEnabled)
        {
            tAccum += dt;
            UpdateGridRadialWave(grid, gridBase, (float)tAccum,
                amplitude, wavelength, speed,
                vertsX, vertsZ);
        }

        // Render
        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    return 0;
}