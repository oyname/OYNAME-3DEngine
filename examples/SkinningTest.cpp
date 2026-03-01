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
#include "core.h"

using namespace DirectX;

// ---------------------------------------------------------------------------
static void CreateCube(LPENTITY* mesh, MATERIAL* material)
{
    LPSURFACE wuerfel = NULL;

    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&wuerfel, (*mesh));

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, -1.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 0.0f, 1.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, -1.0f, 0.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 1.0f, 0.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, -1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, -1.0f, 0.0f);  Engine::VertexColor(wuerfel, 224, 224, 224);

    Engine::AddVertex(wuerfel, -1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, -1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, -1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);
    Engine::AddVertex(wuerfel, 1.0f, 1.0f, 1.0f); Engine::VertexNormal(wuerfel, 0.0f, 1.0f, 0.0f);   Engine::VertexColor(wuerfel, 224, 224, 224);

    // Back (verts 0-3)
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    // Front (verts 4-7)
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    // Left (verts 8-11)
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    // Right (verts 12-15)
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    // Bottom (verts 16-19)
    Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f);
    // Top (verts 20-23)
    Engine::VertexTexCoord(wuerfel, 1.0f, 0.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord(wuerfel, 0.0f, 1.0f);

    Engine::AddTriangle(wuerfel, 0, 1, 2); Engine::AddTriangle(wuerfel, 3, 2, 1);
    Engine::AddTriangle(wuerfel, 6, 5, 4); Engine::AddTriangle(wuerfel, 6, 7, 5);
    Engine::AddTriangle(wuerfel, 8, 9, 10); Engine::AddTriangle(wuerfel, 10, 9, 11);
    Engine::AddTriangle(wuerfel, 14, 13, 12); Engine::AddTriangle(wuerfel, 14, 15, 13);
    Engine::AddTriangle(wuerfel, 16, 17, 18); Engine::AddTriangle(wuerfel, 18, 17, 19);
    Engine::AddTriangle(wuerfel, 21, 22, 23); Engine::AddTriangle(wuerfel, 22, 21, 20);

    Engine::FillBuffer(wuerfel);
}

// ---------------------------------------------------------------------------
static void CreateArmMesh(LPENTITY* mesh, LPMATERIAL material)
{
    LPSURFACE s = nullptr;
    Engine::CreateMesh(mesh, material);
    Engine::CreateSurface(&s, *mesh);

    const float W = 0.5f;
    const XMFLOAT2 corners[4] = { {-W,-W}, { W,-W}, { W, W}, {-W, W} };
    const XMFLOAT3 normals[4] = { {0,0,-1}, {1,0,0}, {0,0,1}, {-1,0,0} };
    const float    ringY[5]   = { -4.f, -2.f, 0.f, 2.f, 4.f };

    // Farbe: Unterarm rot, Oberarm blau
    auto col = [](float y) -> XMFLOAT3 {
        return y <= 0.f ? XMFLOAT3(200,120,120) : XMFLOAT3(120,150,220);
    };

    // 4 Seiten x 5 Ringe x 2 Kanten = 40 Vertices
    std::vector<int> vtxRing;
    for (int side = 0; side < 4; ++side)
    {
        XMFLOAT2 cL = corners[side];
        XMFLOAT2 cR = corners[(side + 1) % 4];
        XMFLOAT3 n  = normals[side];

        for (int ring = 0; ring < 5; ++ring)
        {
            float y = ringY[ring];
            XMFLOAT3 c = col(y);

            Engine::AddVertex(s, cL.x, y, cL.y);
            Engine::VertexNormal(s, n.x, n.y, n.z);
            Engine::VertexColor(s, (int)c.x, (int)c.y, (int)c.z);
            Engine::VertexTexCoord(s, (float)side / 4.f, (y + 4.f) / 8.f);
            Engine::VertexTexCoord2(s, (float)side / 4.f, (y + 4.f) / 8.f);
            vtxRing.push_back(ring);

            Engine::AddVertex(s, cR.x, y, cR.y);
            Engine::VertexNormal(s, n.x, n.y, n.z);
            Engine::VertexColor(s, (int)c.x, (int)c.y, (int)c.z);
            Engine::VertexTexCoord(s, (float)(side + 1) / 4.f, (y + 4.f) / 8.f);
            Engine::VertexTexCoord2(s, (float)(side + 1) / 4.f, (y + 4.f) / 8.f);
            vtxRing.push_back(ring);
        }
    }

    // Bone-Weights: Ring 0+1 = Bone0, Ring 2 = 50/50, Ring 3+4 = Bone1
    for (int v = 0; v < (int)vtxRing.size(); ++v)
    {
        float w0, w1;
        switch (vtxRing[v])
        {
        case 0: case 1: w0 = 1.f; w1 = 0.f; break;
        case 2:         w0 = 0.5f; w1 = 0.5f; break;
        default:        w0 = 0.f; w1 = 1.f; break;
        }
        Engine::VertexBoneData(s, (unsigned int)v, 0, 1, 0, 0, w0, w1, 0.f, 0.f);
    }

    // Indices
    for (int side = 0; side < 4; ++side)
    {
        unsigned int base = side * 10;
        for (int seg = 0; seg < 4; ++seg)
        {
            unsigned int lu = base + seg * 2;
            unsigned int ru = lu + 1;
            unsigned int lo = base + (seg + 1) * 2;
            unsigned int ro = lo + 1;
            Engine::AddTriangle(s, lu, lo, ru);
            Engine::AddTriangle(s, ru, lo, ro);
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

    // Licht
    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::LightColor(light, 1.f, 1.f, 1.f, 0.9f);
    Engine::RotateEntity(light, 45.f, -30.f, 0.f);

    // Kamera
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.f, 0.f, -12.f);

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
    Engine::MaterialColor(matArm, 1.f, 1.f, 1.f, 1.f);

    LPENTITY arm = nullptr;
    CreateArmMesh(&arm, matArm);
    Engine::PositionEntity(arm, 0.f, 0.f, 5.f);

    // Referenzwuerfel (normaler Shader)
    LPMATERIAL matCube = nullptr;
    Engine::CreateMaterial(&matCube);
    Engine::MaterialColor(matCube, 0.6f, 0.6f, 0.6f, 1.f);

    LPENTITY refCube = nullptr;
    CreateCube(&refCube, matCube);
    Engine::PositionEntity(refCube, 4.f, 0.f, 5.f);
    Engine::ScaleEntity(refCube, 0.5f, 0.5f, 0.5f);

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

    while (Windows::MainLoop())
    {
        Core::BeginFrame(); // liefert DeltaTime/FPS/FrameCount über Core

        float dt = (float)Timer::GetDeltaTime();

        boneAngle += 60.f * dt;

        float bend = sinf(boneAngle * 0.01745329f) * 70.f * 0.01745329f;
        bones[1] = XMMatrixRotationZ(bend);

        Engine::SetEntityBoneMatrices(arm, bones, 2);
        Engine::TurnEntity(refCube, 0.f, 30.f * dt, 0.f);
        
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
