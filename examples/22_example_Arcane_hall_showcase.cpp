// arcane_hall_showcase.cpp
//
// Arcane Hall: pulsierende Kristallsaeulen, metallischer Boden mit RTT-Draufsicht,
// orbitierende Wuerfel, rotierende Point Lights.
// Neu: orthografische Top-Down Kamera via SetCameraOrtho -- kein UV-Versatz mehr.

#include "gidx.h"
#include "geometry.h"

#include <cmath>

// -----------------------------------------------------------------------
// Helfer: Box mit PBR-Material
// -----------------------------------------------------------------------
static LPENTITY MakeBox(
    float r, float g, float b, float a,
    float metallic, float roughness,
    float px, float py, float pz,
    float sx, float sy, float sz,
    bool castShadow = true,
    bool receiveShadow = true)
{
    LPMATERIAL mat = nullptr;
    Engine::CreateMaterial(&mat);
    Engine::MaterialColor(mat, r, g, b, a);
    Engine::MaterialUsePBR(mat, true);
    Engine::MaterialMetallic(mat, metallic);
    Engine::MaterialRoughness(mat, roughness);
    Engine::MaterialReceiveShadows(mat, receiveShadow);

    LPENTITY mesh = nullptr;
    CreateCube(&mesh, mat);
    Engine::EntityCastShadows(mesh, castShadow);
    Engine::PositionEntity(mesh, px, py, pz);
    Engine::ScaleEntity(mesh, sx, sy, sz);
    return mesh;
}

// -----------------------------------------------------------------------
// Kristallsaeule
// -----------------------------------------------------------------------
struct Crystal
{
    LPENTITY   mesh;
    LPMATERIAL mat;
    float      r, g, b;
    float      phase;
};

static Crystal MakeCrystal(float r, float g, float b, float phase,
    float px, float py, float pz,
    float sx, float sy, float sz)
{
    LPMATERIAL mat = nullptr;
    Engine::CreateMaterial(&mat);
    Engine::MaterialColor(mat, r * 0.35f, g * 0.35f, b * 0.35f, 0.85f);
    Engine::MaterialUsePBR(mat, true);
    Engine::MaterialMetallic(mat, 0.0f);
    Engine::MaterialRoughness(mat, 0.08f);
    Engine::MaterialTransparent(mat, true);
    Engine::MaterialReceiveShadows(mat, true);
    Engine::MaterialEmissiveColor(mat, r, g, b, 1.5f);

    LPENTITY mesh = nullptr;
    CreateCube(&mesh, mat);
    Engine::EntityCastShadows(mesh, true);
    Engine::PositionEntity(mesh, px, py, pz);
    Engine::ScaleEntity(mesh, sx, sy, sz);

    Crystal c;
    c.mesh = mesh;
    c.mat = mat;
    c.r = r;
    c.g = g;
    c.b = b;
    c.phase = phase;
    return c;
}

// -----------------------------------------------------------------------
// Hauptfunktion
// -----------------------------------------------------------------------
void main(LPVOID hwnd)
{
    Debug::Log("arcane_hall_showcase.cpp: main() gestartet");

    Engine::Graphics(1280, 720, true);

    // ---- Hauptkamera --------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 6.0f, -14.0f);
    Engine::LookAt(camera, 0.0f, 1.5f, 0.0f);

    // ---- Schattenlicht ------------------------------------------------
    LPENTITY sunLight = nullptr;
    Engine::CreateLight(&sunLight, D3DLIGHT_DIRECTIONAL);
    Engine::LightColor(sunLight, 0.90f, 0.82f, 0.70f);
    Engine::PositionEntity(sunLight, 4.0f, 18.0f, -8.0f);
    Engine::LookAt(sunLight, 0.0f, 0.0f, 0.0f);
    Engine::SetDirectionalLight(sunLight);
    Engine::LightShadowOrthoSize(sunLight, 22.0f);
    Engine::LightShadowPlanes(sunLight, 1.0f, 50.0f);

    Engine::SetAmbientColor(0.10f, 0.10f, 0.16f);

    const float CAM_R = 14.0f;
    const float CAM_Y = 6.0f;

    const float FLOOR_SIZE = 15.0f;

    // ---- Boden: PBR Metallic-Spiegel ----------------------------------
    // Planare RTT-Reflexion erfordert projective texture mapping im Shader --
    // ohne den wird die Reflexion immer falsch liegen und wandern.
    // Stattdessen: Metallic=0.95 + Roughness=0.05 -- der PBR Specular-Term
    // ist viewabhaengig und korrekt. Die rotierenden Point Lights erzeugen
    // scharfe, lebendige Lichtflecken die ueberzeugend wie Spiegelungen wirken.
    LPENTITY floorMesh = nullptr;
    {
        LPMATERIAL floorMat = nullptr;
        Engine::CreateMaterial(&floorMat);
        Engine::MaterialColor(floorMat, 0.12f, 0.12f, 0.18f, 1.0f);
        Engine::MaterialUsePBR(floorMat, true);
        Engine::MaterialMetallic(floorMat, 0.95f);
        Engine::MaterialRoughness(floorMat, 0.05f);
        Engine::MaterialReceiveShadows(floorMat, false);
        CreateCube(&floorMesh, floorMat);
        Engine::EntityCastShadows(floorMesh, false);
        Engine::PositionEntity(floorMesh, 0.0f, -0.5f, 0.0f);
        Engine::ScaleEntity(floorMesh, FLOOR_SIZE, 0.4f, FLOOR_SIZE);
    }

    // ---- Zentrale Saeule ----------------------------------------------
    MakeBox(0.10f, 0.10f, 0.14f, 1.0f,
        0.1f, 0.75f,
        0.0f, 2.5f, 0.0f,
        0.9f, 5.0f, 0.9f);

    // ---- Kristallsaeulen im Ring --------------------------------------
    const int   NUM_CRYSTALS = 5;
    const float RING_R = 5.5f;

    struct CrystalDef { float r, g, b; };
    const CrystalDef cDefs[NUM_CRYSTALS] = {
        { 0.1f,  0.9f,  1.0f  },   // Cyan
        { 1.0f,  0.1f,  0.8f  },   // Magenta
        { 1.0f,  0.80f, 0.0f  },   // Gold
        { 0.1f,  1.0f,  0.3f  },   // Gruen
        { 0.35f, 0.35f, 1.0f  },   // Blau
    };

    Crystal crystals[NUM_CRYSTALS];
    for (int i = 0; i < NUM_CRYSTALS; ++i)
    {
        const float angle = i * DirectX::XM_2PI / static_cast<float>(NUM_CRYSTALS);
        const float cx = RING_R * std::sin(angle);
        const float cz = RING_R * std::cos(angle);
        const float phase = i * DirectX::XM_2PI / static_cast<float>(NUM_CRYSTALS);

        crystals[i] = MakeCrystal(
            cDefs[i].r, cDefs[i].g, cDefs[i].b,
            phase,
            cx, 1.5f, cz,
            0.35f, 3.0f, 0.35f);
    }

    // ---- Rotierende Point Lights --------------------------------------
    const int NUM_PLIGHTS = 3;

    struct PLight
    {
        LPENTITY light;
        LPENTITY bulb;   // kleiner leuchtender Wuerfel an der Lichtposition
        float    orbitR, orbitY, speed, angle;
    };

    const float plightDefs[NUM_PLIGHTS][5] = {
        // r      g      b     licht-radius  orbitR
        { 0.1f,  0.9f,  1.0f,  8.0f,  4.0f },   // Cyan
        { 1.0f,  0.1f,  0.8f,  7.0f,  3.2f },   // Magenta
        { 1.0f,  0.80f, 0.0f,  9.0f,  5.0f },   // Gold
    };

    PLight plights[NUM_PLIGHTS];
    for (int i = 0; i < NUM_PLIGHTS; ++i)
    {
        LPENTITY pl = nullptr;
        Engine::CreateLight(&pl, D3DLIGHT_POINT);
        Engine::LightColor(pl,
            plightDefs[i][0], plightDefs[i][1],
            plightDefs[i][2], plightDefs[i][3]);

        plights[i].light = pl;
        plights[i].orbitR = plightDefs[i][4];
        plights[i].orbitY = 1.5f + static_cast<float>(i) * 1.2f;
        plights[i].speed = 0.8f + static_cast<float>(i) * 0.3f;
        plights[i].angle = i * DirectX::XM_2PI / static_cast<float>(NUM_PLIGHTS);

        // Leuchtender Wuerfel an der Lichtposition
        LPMATERIAL bulbMat = nullptr;
        Engine::CreateMaterial(&bulbMat);
        Engine::MaterialColor(bulbMat,
            plightDefs[i][0], plightDefs[i][1], plightDefs[i][2], 1.0f);
        Engine::MaterialUsePBR(bulbMat, true);
        Engine::MaterialMetallic(bulbMat, 0.0f);
        Engine::MaterialRoughness(bulbMat, 0.2f);
        Engine::MaterialReceiveShadows(bulbMat, false);
        Engine::MaterialEmissiveColor(bulbMat,
            plightDefs[i][0], plightDefs[i][1], plightDefs[i][2], 8.0f);

        LPENTITY bulb = nullptr;
        CreateSphere(&bulb, bulbMat, 10, 14, 0.5f);
        Engine::EntityCastShadows(bulb, false);
        plights[i].bulb = bulb;
    }

    // ---- Orbitierende Wuerfel -----------------------------------------
    const int NUM_ORBITERS = 6;

    struct Orbiter
    {
        LPENTITY mesh;
        float    orbitR, orbitY, speed, angle;
    };

    const float orbitColors[NUM_ORBITERS][3] = {
        { 0.90f, 0.90f, 0.95f },
        { 0.75f, 0.75f, 0.90f },
        { 0.85f, 0.82f, 0.80f },
        { 0.70f, 0.70f, 0.75f },
        { 0.92f, 0.88f, 0.82f },
        { 0.78f, 0.78f, 0.85f },
    };

    Orbiter orbiters[NUM_ORBITERS];
    for (int i = 0; i < NUM_ORBITERS; ++i)
    {
        const float r = orbitColors[i][0];
        const float g = orbitColors[i][1];
        const float b = orbitColors[i][2];

        LPMATERIAL mat = nullptr;
        Engine::CreateMaterial(&mat);
        Engine::MaterialColor(mat, r, g, b, 1.0f);
        Engine::MaterialUsePBR(mat, true);
        Engine::MaterialMetallic(mat, 0.80f);
        Engine::MaterialRoughness(mat, 0.15f);
        Engine::MaterialReceiveShadows(mat, true);
        Engine::MaterialEmissiveColor(mat, r * 0.4f, g * 0.4f, b * 0.5f, 0.5f);

        LPENTITY mesh = nullptr;
        CreateCube(&mesh, mat);
        Engine::EntityCastShadows(mesh, true);

        orbiters[i].mesh = mesh;
        orbiters[i].orbitR = 2.5f + static_cast<float>(i % 3) * 0.9f;
        orbiters[i].orbitY = 1.0f + static_cast<float>(i) * 0.6f;
        orbiters[i].speed = 0.6f + static_cast<float>(i) * 0.15f;
        orbiters[i].angle = i * DirectX::XM_2PI / static_cast<float>(NUM_ORBITERS);

        const float sz = 0.25f + static_cast<float>(i % 3) * 0.08f;
        Engine::ScaleEntity(mesh, sz, sz, sz);
    }

    Debug::Log("arcane_hall_showcase.cpp: Szene aufgebaut");

    // ---- Hauptschleife ------------------------------------------------
    float timeAcc = 0.0f;
    float camAngle = 0.0f;

    while (Windows::MainLoop())
    {
        Core::BeginFrame();

        const float dt = static_cast<float>(Timer::GetDeltaTime());
        timeAcc += dt;
        camAngle += 4.0f * dt;

        // Kamera-Orbit
        const float rad = camAngle * DirectX::XM_PI / 180.0f;
        const float cx = CAM_R * std::sin(rad);
        const float cz = -CAM_R * std::cos(rad);
        Engine::PositionEntity(camera, cx, CAM_Y, cz);
        Engine::LookAt(camera, 0.0f, 1.5f, 0.0f);

        // Kristall-Puls
        for (int i = 0; i < NUM_CRYSTALS; ++i)
        {
            const float pulse = 1.9f + 1.3f * std::sin(timeAcc * 1.4f + crystals[i].phase);
            Engine::MaterialEmissiveColor(
                crystals[i].mat,
                crystals[i].r, crystals[i].g, crystals[i].b,
                pulse);
        }

        // Point Lights
        for (int i = 0; i < NUM_PLIGHTS; ++i)
        {
            plights[i].angle += plights[i].speed * dt;
            const float ox = plights[i].orbitR * std::sin(plights[i].angle);
            const float oz = plights[i].orbitR * std::cos(plights[i].angle);
            Engine::PositionEntity(plights[i].light, ox, plights[i].orbitY, oz);
            Engine::PositionEntity(plights[i].bulb, ox, plights[i].orbitY, oz);
        }

        // Orbiter
        for (int i = 0; i < NUM_ORBITERS; ++i)
        {
            orbiters[i].angle += orbiters[i].speed * dt;
            const float ox = orbiters[i].orbitR * std::sin(orbiters[i].angle);
            const float oz = orbiters[i].orbitR * std::cos(orbiters[i].angle);
            Engine::PositionEntity(orbiters[i].mesh, ox, orbiters[i].orbitY, oz);
        }

        Engine::Cls(2, 2, 5);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    Debug::Log("arcane_hall_showcase.cpp: main() beendet");
}
