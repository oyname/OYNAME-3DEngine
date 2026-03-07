// pbr_showcase.cpp
//
// PBR Material Showcase fuer die OYNAME-3DEngine
//
// Zeigt ein 5x2-Grid aus Kugeln:
//   Reihe 0 (oben):  Metallic = 1.0, Roughness variiert von 0.0 bis 1.0
//   Reihe 1 (unten): Metallic = 0.0, Roughness variiert von 0.0 bis 1.0
//
// Jede Kugel teilt dasselbe MeshAsset (ShareMeshAsset) -- Geometrie liegt
// nur einmal im Speicher. Nur das Material unterscheidet sich pro Instanz.
//
// Kamera rotiert langsam um die Szene (Orbit).
//
// Eingebundene API-Aufrufe fuer PBR:
//   MaterialUsePBR(mat, true)
//   MaterialMetallic(mat, 0.0..1.0)
//   MaterialRoughness(mat, 0.0..1.0)
//   MaterialColor(mat, r, g, b)          -- Albedo-Farbe
//   MaterialEmissiveColor(mat, r, g, b)  -- fuer Emissive-Unterlage


#include "gidx.h"
#include "geometry.h"

#include <cmath>
#include <vector>

// -----------------------------------------------------------------------
// UV-Sphere
//
// stacks  = horizontale Ringe (Breitenkreise)
// slices  = vertikale Segmente (Laengenkreise)
// radius  = Radius der Kugel
//
// Baut die Geometrie mit AddVertex/AddTriangle exakt wie CreateCube(),
// vollstaendig mit Normalen und UV-Koordinaten.
// -----------------------------------------------------------------------
static void CreateSphere(LPENTITY* mesh, int stacks, int slices, float radius)
{
    Engine::CreateMesh(mesh);

    LPSURFACE surf = nullptr;
    Engine::CreateSurface(&surf, *mesh);

    // Vertices
    for (int i = 0; i <= stacks; ++i)
    {
        float phi = DirectX::XM_PI * static_cast<float>(i) / static_cast<float>(stacks);
        float y = radius * std::cos(phi);
        float r = radius * std::sin(phi);

        for (int j = 0; j <= slices; ++j)
        {
            float theta = DirectX::XM_2PI * static_cast<float>(j) / static_cast<float>(slices);
            float x = r * std::cos(theta);
            float z = r * std::sin(theta);

            float nx = x / radius;
            float ny = y / radius;
            float nz = z / radius;

            float u = static_cast<float>(j) / static_cast<float>(slices);
            float v = static_cast<float>(i) / static_cast<float>(stacks);

            Engine::AddVertex(surf, x, y, z);
            Engine::VertexNormal(surf, nx, ny, nz);
            Engine::VertexColor(surf, 200, 200, 200);
            Engine::VertexTexCoord(surf, u, v);
        }
    }

    // Indices
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            unsigned int a = static_cast<unsigned int>(i * (slices + 1) + j);
            unsigned int b = static_cast<unsigned int>(i * (slices + 1) + j + 1);
            unsigned int c = static_cast<unsigned int>((i + 1) * (slices + 1) + j);
            unsigned int d = static_cast<unsigned int>((i + 1) * (slices + 1) + j + 1);

            Engine::AddTriangle(surf, a, b, c);
            Engine::AddTriangle(surf, b, d, c);
        }
    }

    Engine::FillBuffer(surf);
}

// -----------------------------------------------------------------------
// Hauptfunktion
// -----------------------------------------------------------------------
void main()
{
    Debug::Log("pbr_showcase.cpp: main() gestartet");

    Engine::Graphics(1280, 720, true);

    // ---- Kamera -----------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 2.0f, -10.0f);
    Engine::LookAt(camera, 0.0f, 0.0f, 0.0f);

    // ---- Licht ------------------------------------------------------
    // Direktionales Licht von oben-links fuer klare PBR-Highlights
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(light, 8.0f, 12.0f, -8.0f);
    Engine::LookAt(light, 0.0f, 0.0f, 0.0f);
    Engine::LightColor(light, 1.0f, 0.97f, 0.90f);  // leicht warmes Weiss
    Engine::SetDirectionalLight(light);
    Engine::SetAmbientColor(0.5f, 0.5f, 0.8f);   // sehr dunkles Blau als Ambient

    // ---- Boden (opak, mattgrau) ------------------------------------
    LPENTITY floor = nullptr;
    Engine::CreateMesh(&floor);
    {
        LPMATERIAL matFloor = nullptr;
        Engine::CreateMaterial(&matFloor);
        Engine::MaterialColor(matFloor, 0.12f, 0.12f, 0.14f, 1.0f);
        Engine::MaterialUsePBR(matFloor, true);
        Engine::MaterialMetallic(matFloor, 0.0f);
        Engine::MaterialRoughness(matFloor, 0.9f);
        CreatePlate(&floor);
        Engine::SurfaceMaterial(Engine::EntitySurface(floor, 0), matFloor);
    }
    Engine::ScaleEntity(floor, 14.0f, 1.0f, 6.0f);
    Engine::PositionEntity(floor, 0.0f, -3.2f, 0.0f);

    // ---- Showcase-Grid aufbauen ------------------------------------
    //
    // 5 Spalten x 2 Reihen = 10 Kugeln
    //
    // Spalte 0..4: Roughness 0.0, 0.25, 0.5, 0.75, 1.0
    // Reihe 0:     Metallic = 1.0  (Gold-Albedo)
    // Reihe 1:     Metallic = 0.0  (Blau-Albedo)
    //
    // Die erste Kugel (0,0) baut die echte Geometrie.
    // Alle weiteren teilen das MeshAsset ueber ShareMeshAsset().
    // Jede Kugel bekommt ihr eigenes Material.

    const int   COLS = 5;
    const int   ROWS = 2;
    const float SPACING = 3.0f;
    const float RADIUS = 1.0f;

    // Roughness-Werte pro Spalte
    const float roughness[COLS] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };

    // Albedo pro Reihe: Reihe 0 = warmes Gold, Reihe 1 = kuehliges Blau
    const float albedo[ROWS][3] = {
        { 1.0f, 0.78f, 0.34f },   // Gold (typisch fuer Metall-PBR)
        { 0.10f, 0.35f, 0.85f }   // Blau (Dielectric/Nichtmetall)
    };
    const float metallic[ROWS] = { 1.0f, 0.0f };

    // Offset damit das Grid mittig sitzt
    float totalWidth = (COLS - 1) * SPACING;
    float totalHeight = (ROWS - 1) * SPACING;
    float startX = -totalWidth * 0.5f;
    float startY = totalHeight * 0.5f;

    // Erste Kugel aufbauen (Referenz-Geometrie)
    LPENTITY sphereRef = nullptr;
    CreateSphere(&sphereRef, 24, 32, RADIUS);

    // Array fuer alle Instanzen (10 Stueck)
    LPENTITY spheres[ROWS][COLS] = {};

    for (int row = 0; row < ROWS; ++row)
    {
        for (int col = 0; col < COLS; ++col)
        {
            float posX = startX + col * SPACING;
            float posY = startY - row * SPACING;

            LPENTITY sphere = nullptr;

            if (row == 0 && col == 0)
            {
                // Erste Kugel = Referenz-Geometrie, direkt verwenden
                sphere = sphereRef;
            }
            else
            {
                // Neue Mesh-Instanz, Geometrie von sphereRef teilen
                Engine::CreateMesh(&sphere);
                Engine::ShareMeshAsset(sphereRef, sphere);
            }

            // Eigenes PBR-Material pro Kugel
            LPMATERIAL mat = nullptr;
            Engine::CreateMaterial(&mat);
            Engine::MaterialUsePBR(mat, true);
            Engine::MaterialColor(mat,
                albedo[row][0],
                albedo[row][1],
                albedo[row][2],
                1.0f);
            Engine::MaterialMetallic(mat, metallic[row]);
            Engine::MaterialRoughness(mat, roughness[col]);

            // Kugeln am Rand mit sehr glatter Oberflaeche bekommen
            // einen dezenten Emissive-Schimmer damit die Highlights
            // besser erkennbar sind
            if (roughness[col] < 0.1f)
            {
                Engine::MaterialEmissiveColor(mat,
                    albedo[row][0] * 0.15f,
                    albedo[row][1] * 0.15f,
                    albedo[row][2] * 0.15f,
                    1.0f);
            }

            Engine::SetSlotMaterial(sphere, 0, mat);
            Engine::PositionEntity(sphere, posX, posY, 0.0f);

            spheres[row][col] = sphere;

            Debug::Log("pbr_showcase.cpp: Kugel [row=", row, " col=", col, "]"
                " metallic=", metallic[row],
                " roughness=", roughness[col],
                " pos=(", posX, ",", posY, ",0)");
        }
    }

    // ---- Emissive-Unterlage (Label-Streifen) -------------------------
    // Zwei schmale Platten unter den Reihen als visuelle Trennlinie.
    // Emissive damit sie auch ohne direktes Licht leuchten.
    for (int row = 0; row < ROWS; ++row)
    {
        LPENTITY bar = nullptr;
        Engine::CreateMesh(&bar);

        LPMATERIAL matBar = nullptr;
        Engine::CreateMaterial(&matBar);
        Engine::MaterialUsePBR(matBar, false);
        Engine::MaterialColor(matBar, 0.05f, 0.05f, 0.05f, 1.0f);
        Engine::MaterialEmissiveColor(matBar,
            albedo[row][0] * 0.5f,
            albedo[row][1] * 0.5f,
            albedo[row][2] * 0.5f,
            0.6f);

        CreatePlate(&bar);
        Engine::SurfaceMaterial(Engine::EntitySurface(bar, 0), matBar);
        Engine::ScaleEntity(bar, totalWidth * 0.5f + SPACING * 0.5f, 1.0f, 0.08f);
        Engine::PositionEntity(bar,
            0.0f,
            startY - row * SPACING - RADIUS - 0.3f,
            0.0f);
    }

    Debug::Log("pbr_showcase.cpp: Szene aufgebaut - ", ROWS * COLS, " Kugeln, 1 Referenz-Asset");

    // ---- Orbit-Parameter --------------------------------------------
    float   orbitAngle = 0.0f;
    float   orbitRadius = 10.0f;
    float   orbitHeight = 2.5f;
    float   orbitSpeed = 12.0f;   // Grad pro Sekunde

    // ---- Hauptschleife ----------------------------------------------
    while (Windows::MainLoop())
    {
        Core::BeginFrame();

        float dt = static_cast<float>(Timer::GetDeltaTime());

        // Kamera langsam um die Szene kreisen lassen
        orbitAngle += orbitSpeed * dt;
        if (orbitAngle >= 360.0f) orbitAngle -= 360.0f;

        float rad = orbitAngle * DirectX::XM_PI / 180.0f;
        float camX = orbitRadius * std::sin(rad);
        float camZ = -(orbitRadius * std::cos(rad));

        Engine::PositionEntity(camera, camX, orbitHeight, camZ);
        Engine::LookAt(camera, 0.0f, 0.0f, 0.0f);

        Engine::Cls(8, 8, 12);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    Debug::Log("pbr_showcase.cpp: main() beendet");
}
