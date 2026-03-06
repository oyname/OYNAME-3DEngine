// SkinningTest.cpp
// Testet den Skinning-Vertex-Shader mit einem handgebauten Arm-Mesh.
//
// Aufbau:
//   Bone 0 = Unterarm (Y -4..0), bleibt statisch
//   Bone 1 = Oberarm  (Y 0..+4), rotiert um Z am Gelenk Y=0
//   Ring bei Y=0 = gemischte Weights 0.5/0.5
//
// Steuerung: ESC = Beenden

#include "gidx.h"
#include "geometry.h"

using namespace DirectX;

// ---------------------------------------------------------------------------
static void CreateArmMesh(LPENTITY* mesh, LPMATERIAL material)
{
    LPSURFACE s = nullptr;
    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&s, *mesh);

    // SurfaceMaterial muss explizit das Material übergeben welchen den Shader für Animation nutzt
    //
    Engine::SurfaceMaterial(s, material);

    const float W = 0.5f;

    // corners im XZ-Plane (x,z)
    const XMFLOAT2 corners[4] = { {-W,-W}, { W,-W}, { W, W}, {-W, W} };

    // gewünschte OUTWARD-Normalen pro Seite (für den Mantel)
    // side0: von corner0->1, sollte nach -Z zeigen
    // side1: nach +X
    // side2: nach +Z
    // side3: nach -X
    const XMFLOAT3 outward[4] = { {0,0,-1}, {1,0,0}, {0,0,1}, {-1,0,0} };

    const float ringY[5] = { -4.f, -2.f, 0.f, 2.f, 4.f };

    auto col = [](float y) -> XMFLOAT3 {
        return y <= 0.f ? XMFLOAT3(200, 120, 120) : XMFLOAT3(120, 150, 220);
        };

    auto clamp01 = [](float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); };

    // Wir speichern die Positionsdaten parallel, damit wir Winding checken können.
    std::vector<XMFLOAT3> pos;
    pos.reserve(4 * 5 * 2);

    std::vector<int> vtxRing;
    vtxRing.reserve(4 * 5 * 2);

    // --- Vertices bauen ---
    for (int side = 0; side < 4; ++side)
    {
        XMFLOAT2 cL = corners[side];
        XMFLOAT2 cR = corners[(side + 1) % 4];
        XMFLOAT3 n = outward[side];

        for (int ring = 0; ring < 5; ++ring)
        {
            float y = ringY[ring];
            XMFLOAT3 c = col(y);

            // links
            Engine::AddVertex(s, cL.x, y, cL.y);
            Engine::VertexNormal(s, n.x, n.y, n.z);
            Engine::VertexColor(s, (int)c.x, (int)c.y, (int)c.z);
            Engine::VertexTexCoord(s, (float)side / 4.f, (y + 4.f) / 8.f);
            Engine::VertexTexCoord2(s, (float)side / 4.f, (y + 4.f) / 8.f);
            pos.push_back(XMFLOAT3(cL.x, y, cL.y));
            vtxRing.push_back(ring);

            // rechts
            Engine::AddVertex(s, cR.x, y, cR.y);
            Engine::VertexNormal(s, n.x, n.y, n.z);
            Engine::VertexColor(s, (int)c.x, (int)c.y, (int)c.z);
            Engine::VertexTexCoord(s, (float)(side + 1) / 4.f, (y + 4.f) / 8.f);
            Engine::VertexTexCoord2(s, (float)(side + 1) / 4.f, (y + 4.f) / 8.f);
            pos.push_back(XMFLOAT3(cR.x, y, cR.y));
            vtxRing.push_back(ring);
        }
    }

    // --- Bone Weights ---
    for (int v = 0; v < (int)vtxRing.size(); ++v)
    {
        float y = -4.f + 2.f * (float)vtxRing[v];

        float k = 2.0f;
        float t = (y + k) / (2.0f * k);
        t = clamp01(t);
        t = t * t * (3.f - 2.f * t); // smoothstep

        float w1 = t;
        float w0 = 1.f - t;

        Engine::VertexBoneData(s, (unsigned int)v, 0, 1, 0, 0, w0, w1, 0.f, 0.f);
    }

    // Hilfsfunktionen für Winding-Test
    auto sub = [](const XMFLOAT3& a, const XMFLOAT3& b) -> XMFLOAT3 {
        return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
        };
    auto cross = [](const XMFLOAT3& a, const XMFLOAT3& b) -> XMFLOAT3 {
        return XMFLOAT3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
        };
    auto dot = [](const XMFLOAT3& a, const XMFLOAT3& b) -> float {
        return a.x * b.x + a.y * b.y + a.z * b.z;
        };

    // --- Indices: pro Seite Winding automatisch korrekt setzen ---
    for (int side = 0; side < 4; ++side)
    {
        const unsigned int base = side * 10; // 5 Ringe * 2 verts
        const XMFLOAT3 desiredN = outward[side];

        // Winding anhand des ersten Quads testen (Seg 0)
        unsigned int lu0 = base + 0;
        unsigned int ru0 = base + 1;
        unsigned int lo0 = base + 2;
        // Normal aus (lu,ru,lo)
        XMFLOAT3 e1 = sub(pos[ru0], pos[lu0]);
        XMFLOAT3 e2 = sub(pos[lo0], pos[lu0]);
        XMFLOAT3 triN = cross(e1, e2);

        bool flip = (dot(triN, desiredN) < 0.0f);

        for (int seg = 0; seg < 4; ++seg)
        {
            unsigned int lu = base + seg * 2;
            unsigned int ru = lu + 1;
            unsigned int lo = base + (seg + 1) * 2;
            unsigned int ro = lo + 1;

            if (!flip)
            {
                // (lu,ru,lo) (ru,ro,lo)
                Engine::AddTriangle(s, lu, ru, lo);
                Engine::AddTriangle(s, ru, ro, lo);
            }
            else
            {
                // flipped
                Engine::AddTriangle(s, lu, lo, ru);
                Engine::AddTriangle(s, ru, lo, ro);
            }
        }
    }

    Engine::FillBuffer(s);
    Debug::Log("SkinningTest.cpp: CreateArmMesh - Vertices: ", (int)vtxRing.size());
}

// ---------------------------------------------------------------------------
int main()
{
    // DirectX initialisieren -- MUSS als Erstes aufgerufen werden
    Engine::Graphics(1024, 768);  // 0/0 = Desktop-Aufloesung

    ///////////////////////////
        // -------------------------------------------------
    // Textur laden
    // -------------------------------------------------
    LPTEXTURE faceTex = nullptr;
    LPTEXTURE albedoTex = nullptr;
    LPTEXTURE normalTex = nullptr;
    LPTEXTURE ormTex = nullptr;
    Engine::LoadTexture(&faceTex, L"..\\media\\engine.png");
    Engine::LoadTexture(&albedoTex, L"..\\media\\albedo.png");
    Engine::LoadTexture(&normalTex, L"..\\media\\normal.png");
    Engine::LoadTexture(&ormTex, L"..\\media\\orm.png");

    // -------------------------------------------------
    // Material A erstellen
    // -------------------------------------------------
    LPMATERIAL matCube = nullptr;
    Engine::CreateMaterial(&matCube);

    Debug::Log("Material matCube: ", matCube);

    Engine::MaterialTexture(matCube, albedoTex, 0);
    Engine::MaterialTexture(matCube, normalTex, 1);
    Engine::MaterialTexture(matCube, ormTex, 2);

    // Basisfarbe
    Engine::MaterialColor(matCube, 1.0f, 1.0f, 1.0f, 1.0f);

    // PBR Parameter
    Engine::MaterialMetallic(matCube, 0.5f);
    Engine::MaterialRoughness(matCube, 0.4f);
    Engine::MaterialNormalScale(matCube, 1.5f);
    Engine::MaterialOcclusionStrength(matCube, 0.3f);
    ////////////////////////////////////////////////////

    // Licht
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(light, 0.0f, 10.0f, 0.0f);
    Engine::LightColor(light, 1.f, 1.f, 1.f, 0.9f);
    Engine::RotateEntity(light, -30.0f, 0.f, 45.f);

    // Kamera
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.f, 0.f, -10.f);

    // Skinning-Shader
    DWORD skinFlags = D3DVERTEX_POSITION | D3DVERTEX_NORMAL | D3DVERTEX_COLOR
                     | D3DVERTEX_TEX1 | D3DVERTEX_TEX2
                     | D3DVERTEX_TANGENT
                     | D3DVERTEX_BONE_INDICES | D3DVERTEX_BONE_WEIGHTS;

    LPSHADER skinShader = nullptr;
    Engine::CreateShader(&skinShader,
        L"..\\shaders\\VertexShaderSkinning.hlsl", "main",
        L"..\\shaders\\PixelShader.hlsl",          "main",
        skinFlags);


    // Arm-Material + Mesh
    LPMATERIAL matArm = nullptr;
    Engine::CreateMaterial(&matArm, skinShader);
    Debug::Log("Material matArm: ", matArm);
    //Engine::MaterialColor(matArm, 1.f, 1.f, 1.f, 1.f);
    Engine::MaterialTexture(matArm, albedoTex, 0);
    Engine::MaterialTexture(matArm, normalTex, 1);
    Engine::MaterialTexture(matArm, ormTex, 2);

    // Basisfarbe
    Engine::MaterialColor(matArm, 1.0f, 1.0f, 1.0f, 1.0f);

    // PBR Parameter
    Engine::MaterialMetallic(matArm, 0.5f);
    Engine::MaterialRoughness(matArm, 0.4f);
    Engine::MaterialNormalScale(matArm, 1.5f);
    Engine::MaterialOcclusionStrength(matArm, 0.3f);

    // Referenzwuerfel (normaler Shader)
    LPENTITY refCube = nullptr;
    CreateCube(&refCube, matCube);
    Engine::SurfaceMaterial(Engine::GetSurface(refCube), matCube);
    Engine::PositionEntity(refCube, 4.f, 0.f, 0.f);
    Engine::ScaleEntity(refCube, 1.0f, 1.0f, 1.0f);

    // Arm (Animations Shader)
    LPENTITY arm = nullptr;
    CreateArmMesh(&arm, matArm);
    Engine::PositionEntity(arm, 0.f, 0.f, 5.f);

    // Bone-Matrizen
    XMMATRIX bones[2];
    float boneAngle = 0.f;
    // Winkel/Animation
    float bend = sinf(boneAngle) * 1.2f; // Radiant

    // Pivot (Gelenk) – bei dir Y=0
    float pivotY = 0.0f;

    XMMATRIX T0 = XMMatrixTranslation(0.f, pivotY, 0.f);
    XMMATRIX T1 = XMMatrixTranslation(0.f, -pivotY, 0.f);

    bones[0] = XMMatrixIdentity();
    bones[1] = T0 * XMMatrixRotationZ(bend) * T1;


    Debug::Log("SkinningTest.cpp: Start");

    Engine::DebugPrintScene();

    while (Windows::MainLoop())
    {
        Core::BeginFrame(); // liefert DeltaTime/FPS/FrameCount über Core

        float dt = (float)Timer::GetDeltaTime();

        boneAngle += 60.f * dt;

        float bend = sinf(boneAngle * 0.01745329f) * 70.f * 0.01745329f;
        bones[1] = T0 * XMMatrixRotationZ(bend) * T1;

        Engine::SetEntityBoneMatrices(arm, bones, 2);
        Engine::TurnEntity(arm, 0.f, 30.f * dt, 0.f);
        Engine::TurnEntity(refCube, 30.f * dt, 30.f * dt, 0.f);
        
        Engine::Cls(0, 64, 128);
        
        Engine::UpdateWorld();

        HRESULT hr = Engine::RenderWorld();
        if (FAILED(hr))
            Debug::Log("FEHLER: RenderWorld");

        hr = Engine::Flip();
        if (FAILED(hr))
            Debug::Log("FEHLER: Flip");

        Core::EndFrame();
    }

    return 0;
}
