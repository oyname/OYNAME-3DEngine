// water_showcase.cpp
//
// Wasser-Oberfläche mit animierten Wellen.
// Drei rote Bojen schwimmen auf den Wellen mit.

#include "gidx.h"
#include "geometry.h"

#include <cmath>
#include <vector>

// -----------------------------------------------------------------------
// Grid-Vertex-Basisdaten (X/Z statisch, Y wird animiert)
// -----------------------------------------------------------------------
struct GridBase { float x; float z; };

static inline int VIdx(int x, int z, int vertsX) { return z * vertsX + x; }

// Wellenparameter -- identisch in AnimateWater und WaveHeight
static const float WK1 = DirectX::XM_2PI / 8.0f;
static const float WK2 = DirectX::XM_2PI / 5.0f;
static const float WS1 = 1.2f;
static const float WS2 = 1.8f;
static const float WA1 = 0.18f;
static const float WA2 = 0.10f;
static const float WD1X = 0.70f, WD1Z = 0.70f;
static const float WD2X = -0.40f, WD2Z = 0.92f;

// -----------------------------------------------------------------------
// Wellenhöhe an beliebiger Weltposition (für Bojen)
// -----------------------------------------------------------------------
static float WaveHeight(float px, float pz, float t)
{
    const float phase1 = WK1 * (WD1X * px + WD1Z * pz) - WS1 * t;
    const float phase2 = WK2 * (WD2X * px + WD2Z * pz) - WS2 * t;
    return WA1 * std::sin(phase1) + WA2 * std::sin(phase2);
}

// -----------------------------------------------------------------------
// Gitter aufbauen -- gleiche Struktur wie das funktionierende Beispiel
// -----------------------------------------------------------------------
static void CreateWaterGrid(
    LPENTITY* mesh,
    LPMATERIAL material,
    int cellsX, int cellsZ,
    float sizeX, float sizeZ,
    std::vector<GridBase>& outBase)
{
    Engine::CreateMesh(mesh);

    LPSURFACE surface = nullptr;
    Engine::CreateSurface(&surface, *mesh);
    Engine::SetSlotMaterial(*mesh, 0, material);   // Slot-Material VOR FillBuffer

    const int vertsX = cellsX + 1;
    const int vertsZ = cellsZ + 1;
    const int vertCount = vertsX * vertsZ;

    outBase.assign(vertCount, { 0.0f, 0.0f });

    const float halfX = sizeX * 0.5f;
    const float halfZ = sizeZ * 0.5f;

    int vid = 0;
    for (int z = 0; z < vertsZ; ++z)
    {
        const float tz = static_cast<float>(z) / static_cast<float>(cellsZ);
        const float wz = -halfZ + tz * sizeZ;

        for (int x = 0; x < vertsX; ++x)
        {
            const float tx = static_cast<float>(x) / static_cast<float>(cellsX);
            const float wx = -halfX + tx * sizeX;

            outBase[vid] = { wx, wz };

            Engine::AddVertex(vid, surface, DirectX::XMFLOAT3(wx, 0.0f, wz));
            Engine::VertexNormal(surface, 0.0f, 1.0f, 0.0f);
            Engine::VertexColor(surface, 100, 180, 255);
            Engine::VertexTexCoord(surface, tx, tz);

            ++vid;
        }
    }

    for (int z = 0; z < cellsZ; ++z)
    {
        for (int x = 0; x < cellsX; ++x)
        {
            const int i0 = VIdx(x, z, vertsX);
            const int i1 = VIdx(x + 1, z, vertsX);
            const int i2 = VIdx(x, z + 1, vertsX);
            const int i3 = VIdx(x + 1, z + 1, vertsX);

            Engine::AddTriangle(surface, i0, i2, i1);
            Engine::AddTriangle(surface, i1, i2, i3);
        }
    }

    Engine::FillBuffer(*mesh, 0);
}

// -----------------------------------------------------------------------
// Wellen-Animation: Superposition zweier gerichteter Sinuswellen
// -----------------------------------------------------------------------
static void AnimateWater(
    LPENTITY mesh,
    const std::vector<GridBase>& base,
    float t,
    int vertsX, int vertsZ)
{
    LPSURFACE surface = Engine::GetSurface(mesh);

    for (int z = 0; z < vertsZ; ++z)
    {
        for (int x = 0; x < vertsX; ++x)
        {
            const int   idx = VIdx(x, z, vertsX);
            const float px = base[idx].x;
            const float pz = base[idx].z;

            const float phase1 = WK1 * (WD1X * px + WD1Z * pz) - WS1 * t;
            const float phase2 = WK2 * (WD2X * px + WD2Z * pz) - WS2 * t;

            const float y = WA1 * std::sin(phase1) + WA2 * std::sin(phase2);

            Engine::AddVertex(idx, surface, DirectX::XMFLOAT3(px, y, pz));

            const float dydx = WA1 * WK1 * WD1X * std::cos(phase1)
                + WA2 * WK2 * WD2X * std::cos(phase2);
            const float dydz = WA1 * WK1 * WD1Z * std::cos(phase1)
                + WA2 * WK2 * WD2Z * std::cos(phase2);
            const float nx = -dydx;
            const float nz = -dydz;
            const float len = std::sqrt(nx * nx + 1.0f + nz * nz);
            Engine::VertexNormal(surface, idx, nx / len, 1.0f / len, nz / len);
        }
    }

    Engine::UpdateVertexBuffer(surface);
    Engine::UpdateNormalBuffer(surface);
}

// -----------------------------------------------------------------------
// Helfer: opake Box
// -----------------------------------------------------------------------
static LPENTITY MakeBox(float r, float g, float b,
    float px, float py, float pz,
    float sx, float sy, float sz)
{
    LPMATERIAL mat = nullptr;
    Engine::CreateMaterial(&mat);
    Engine::MaterialColor(mat, r, g, b, 1.0f);
    Engine::MaterialUsePBR(mat, true);
    Engine::MaterialMetallic(mat, 0.0f);
    Engine::MaterialRoughness(mat, 0.65f);
    Engine::MaterialReceiveShadows(mat, true);

    LPENTITY mesh = nullptr;
    CreateCube(&mesh, mat);
    Engine::EntityCastShadows(mesh, true);
    Engine::PositionEntity(mesh, px, py, pz);
    Engine::ScaleEntity(mesh, sx, sy, sz);
    return mesh;
}

// -----------------------------------------------------------------------
// Hauptfunktion
// -----------------------------------------------------------------------
void main(LPVOID hwnd)
{
    Debug::Log("water_showcase.cpp: main() gestartet");

    Engine::Graphics(1280, 720, true);

    // ---- Kamera -------------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 8.0f, -14.0f);
    Engine::LookAt(camera, 0.0f, 0.0f, 0.0f);

    // ---- Licht --------------------------------------------------------
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::LightColor(light, 1.0f, 0.95f, 0.85f);
    Engine::PositionEntity(light, 8.0f, 14.0f, -6.0f);
    Engine::LookAt(light, 0.0f, 0.0f, 0.0f);
    Engine::SetDirectionalLight(light);
    Engine::LightShadowOrthoSize(light, 24.0f);
    Engine::LightShadowPlanes(light, 1.0f, 60.0f);
    Engine::SetAmbientColor(0.15f, 0.20f, 0.30f);

    // ---- Szene (Leuchtturm-Koloss links) --------------------------------
    MakeBox(0.72f, 0.66f, 0.60f, -5.0f, 1.0f, 2.0f, 1.0f, 2.0f, 1.0f);
    MakeBox(0.60f, 0.55f, 0.50f, -5.0f, 3.5f, 2.0f, 0.7f, 1.5f, 0.7f);
    MakeBox(0.50f, 0.45f, 0.42f, -5.0f, 5.5f, 2.0f, 0.5f, 1.0f, 0.5f);

    LPENTITY beacon = MakeBox(1.0f, 0.7f, 0.2f, -5.0f, 7.0f, 2.0f, 0.3f, 0.3f, 0.3f);
    {
        LPMATERIAL matB = nullptr;
        Engine::CreateMaterial(&matB);
        Engine::MaterialUsePBR(matB, true);
        Engine::MaterialEmissiveColor(matB, 1.0f, 0.65f, 0.1f, 3.0f);
        Engine::MaterialMetallic(matB, 0.0f);
        Engine::MaterialRoughness(matB, 0.3f);
        Engine::MaterialReceiveShadows(matB, false);
        Engine::SetSlotMaterial(beacon, 0, matB);
    }

    MakeBox(0.40f, 0.38f, 0.35f, -5.0f, -0.5f, 2.0f, 2.5f, 0.5f, 2.5f);

    // ---- Bojen (rote Würfel, schwimmen auf den Wellen) -----------------
    struct Boje { LPENTITY mesh; float bx; float bz; float baseY; };

    const Boje bojen[] = {
        { MakeBox(0.85f, 0.20f, 0.20f,  3.5f, 0.4f, -1.0f, 0.35f, 0.35f, 0.35f),  3.5f, -1.0f, 0.4f },
        { MakeBox(0.85f, 0.20f, 0.20f,  5.0f, 0.2f,  1.5f, 0.30f, 0.30f, 0.30f),  5.0f,  1.5f, 0.2f },
        { MakeBox(0.85f, 0.20f, 0.20f,  2.5f, 0.3f,  3.5f, 0.32f, 0.32f, 0.32f),  2.5f,  3.5f, 0.3f },
    };

    // ---- Wasser-Material ----------------------------------------------
    LPMATERIAL waterMat = nullptr;
    Engine::CreateMaterial(&waterMat);
    Engine::MaterialColor(waterMat, 0.05f, 0.25f, 0.55f, 0.75f);
    Engine::MaterialUsePBR(waterMat, true);
    Engine::MaterialMetallic(waterMat, 0.0f);
    Engine::MaterialRoughness(waterMat, 0.08f);
    Engine::MaterialTransparent(waterMat, true);
    Engine::MaterialReceiveShadows(waterMat, true);

    // ---- Wasser-Gitter ------------------------------------------------
    const int   CELLS = 32;
    const float SIZE = 16.0f;
    const int   VERTS = CELLS + 1;

    LPENTITY waterMesh = nullptr;
    std::vector<GridBase> gridBase;
    CreateWaterGrid(&waterMesh, waterMat, CELLS, CELLS, SIZE, SIZE, gridBase);
    Engine::EntityCastShadows(waterMesh, false);

    Debug::Log("water_showcase.cpp: Szene aufgebaut");

    // ---- Hauptschleife ------------------------------------------------
    float timeAcc = 0.0f;
    float camAngle = 20.0f;
    const float CAM_R = 15.0f;
    const float CAM_Y = 8.0f;

    while (Windows::MainLoop())
    {
        Core::BeginFrame();

        const float dt = static_cast<float>(Timer::GetDeltaTime());
        timeAcc += dt;
        camAngle += 3.5f * dt;

        const float rad = camAngle * DirectX::XM_PI / 180.0f;
        const float cx = CAM_R * std::sin(rad);
        const float cz = -CAM_R * std::cos(rad);
        Engine::PositionEntity(camera, cx, CAM_Y, cz);
        Engine::LookAt(camera, 0.0f, 0.5f, 0.0f);

        // Bojen auf Wellenhoehe heben/senken
        for (const Boje& b : bojen)
        {
            const float wy = WaveHeight(b.bx, b.bz, timeAcc);
            Engine::PositionEntity(b.mesh, b.bx, b.baseY + wy, b.bz);
        }

        AnimateWater(waterMesh, gridBase, timeAcc, VERTS, VERTS);

        Engine::Cls(8, 12, 22);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    Debug::Log("water_showcase.cpp: main() beendet");
}