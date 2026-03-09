// skinned_standard_showcase.cpp
// Demonstriert den gemeinsamen Standard-Pixelshader fuer statische und skinned Meshes.
// Static Mesh  -> normales Material (CreateMaterial)
// Skinned Mesh -> skinned Standardmaterial (CreateSkinnedMaterial)

#include "gidx.h"
#include "geometry.h"
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

static void CreateArmMesh(LPENTITY* mesh, LPMATERIAL material)
{
    LPSURFACE s = nullptr;
    Engine::CreateMesh(mesh);
    Engine::CreateSurface(&s, *mesh);
    if (material) Engine::SetSlotMaterial(*mesh, 0, material);

    const float W = 0.5f;
    const XMFLOAT2 corners[4] = { {-W,-W}, { W,-W}, { W, W}, {-W, W} };
    const XMFLOAT3 outward[4] = { {0,0,-1}, {1,0,0}, {0,0,1}, {-1,0,0} };
    const float ringY[5] = { -4.f, -2.f, 0.f, 2.f, 4.f };

    auto clamp01 = [](float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); };

    std::vector<XMFLOAT3> pos;
    pos.reserve(4 * 5 * 2);
    std::vector<int> vtxRing;
    vtxRing.reserve(4 * 5 * 2);

    for (int side = 0; side < 4; ++side)
    {
        XMFLOAT2 cL = corners[side];
        XMFLOAT2 cR = corners[(side + 1) % 4];
        XMFLOAT3 n = outward[side];

        for (int ring = 0; ring < 5; ++ring)
        {
            float y = ringY[ring];
            float u0 = (float)side / 4.f;
            float u1 = (float)(side + 1) / 4.f;
            float v = (y + 4.f) / 8.f;

            Engine::AddVertex(s, cL.x, y, cL.y);
            Engine::VertexNormal(s, n.x, n.y, n.z);
            Engine::VertexColor(s, 255, 255, 255);
            Engine::VertexTexCoord(s, u0, v);
            Engine::VertexTexCoord2(s, u0, v);
            pos.push_back(XMFLOAT3(cL.x, y, cL.y));
            vtxRing.push_back(ring);

            Engine::AddVertex(s, cR.x, y, cR.y);
            Engine::VertexNormal(s, n.x, n.y, n.z);
            Engine::VertexColor(s, 255, 255, 255);
            Engine::VertexTexCoord(s, u1, v);
            Engine::VertexTexCoord2(s, u1, v);
            pos.push_back(XMFLOAT3(cR.x, y, cR.y));
            vtxRing.push_back(ring);
        }
    }

    for (int v = 0; v < (int)vtxRing.size(); ++v)
    {
        float y = -4.f + 2.f * (float)vtxRing[v];
        float k = 2.0f;
        float t = (y + k) / (2.0f * k);
        t = clamp01(t);
        t = t * t * (3.f - 2.f * t);

        float w1 = t;
        float w0 = 1.f - t;
        Engine::VertexBoneData(s, (unsigned int)v, 0, 1, 0, 0, w0, w1, 0.f, 0.f);
    }

    auto sub = [](const XMFLOAT3& a, const XMFLOAT3& b) -> XMFLOAT3 {
        return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
    };
    auto cross = [](const XMFLOAT3& a, const XMFLOAT3& b) -> XMFLOAT3 {
        return XMFLOAT3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x);
    };
    auto dot = [](const XMFLOAT3& a, const XMFLOAT3& b) -> float {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    };

    for (int side = 0; side < 4; ++side)
    {
        const unsigned int base = side * 10;
        const XMFLOAT3 desiredN = outward[side];

        unsigned int lu0 = base + 0;
        unsigned int ru0 = base + 1;
        unsigned int lo0 = base + 2;
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
                Engine::AddTriangle(s, lu, ru, lo);
                Engine::AddTriangle(s, ru, ro, lo);
            }
            else
            {
                Engine::AddTriangle(s, lu, lo, ru);
                Engine::AddTriangle(s, ru, lo, ro);
            }
        }
    }

    Engine::FillBuffer(*mesh, 0);
}

int main()
{
    Engine::Graphics(1280, 720);

    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.f, 1.0f, -7.f);
    Engine::LookAt(camera, 0.f, 0.f, 5.f);

    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(light, 10.0f, 0.0f, -10.0f);
    Engine::LookAt(light, 0.0f, 0.0f, 0.0f);
    Engine::LightColor(light, 2.f, 2.f, 2.5f);
    Engine::SetDirectionalLight(light);
    Engine::SetAmbientColor(0.4f, 0.4f, 0.54f);

    LPTEXTURE albedoTex = nullptr;
    LPTEXTURE normalTex = nullptr;
    LPTEXTURE ormTex = nullptr;
    Engine::LoadTexture(&albedoTex, L"..\\media\\albedo.png");
    Engine::LoadTexture(&normalTex, L"..\\media\\normal.png");
    Engine::LoadTexture(&ormTex,    L"..\\media\\orm.png");

    LPMATERIAL staticMat = nullptr;
    Engine::CreateMaterial(&staticMat);
    Engine::MaterialUsePBR(staticMat, true);
    Engine::MaterialTexture(staticMat, albedoTex, 0);
    Engine::MaterialTexture(staticMat, normalTex, 1);
    Engine::MaterialTexture(staticMat, ormTex, 2);
    Engine::MaterialColor(staticMat, 1.f, 1.f, 1.f, 1.f);
    Engine::MaterialMetallic(staticMat, 0.3f);
    Engine::MaterialRoughness(staticMat, 0.45f);
    Engine::MaterialNormalScale(staticMat, 1.0f);

    LPMATERIAL skinnedMat = nullptr;
    Engine::CreateSkinnedMaterial(&skinnedMat);
    Engine::MaterialUsePBR(skinnedMat, true);
    Engine::MaterialTexture(skinnedMat, albedoTex, 0);
    Engine::MaterialTexture(skinnedMat, normalTex, 1);
    Engine::MaterialTexture(skinnedMat, ormTex, 2);
    Engine::MaterialColor(skinnedMat, 1.f, 1.f, 1.f, 1.f);
    Engine::MaterialMetallic(skinnedMat, 0.3f);
    Engine::MaterialRoughness(skinnedMat, 0.45f);
    Engine::MaterialNormalScale(skinnedMat, 1.0f);

    LPENTITY cube = nullptr;
    CreateCube(&cube, staticMat);
    Engine::PositionEntity(cube, 4.f, 0.f, 4.f);

    LPENTITY arm = nullptr;
    CreateArmMesh(&arm, skinnedMat);
    Engine::PositionEntity(arm, 0.f, 0.f, 6.f);

    XMMATRIX bones[2];
    bones[0] = XMMatrixIdentity();
    float boneAngle = 0.f;
    const XMMATRIX T0 = XMMatrixTranslation(0.f, 0.f, 0.f);
    const XMMATRIX T1 = XMMatrixTranslation(0.f, 0.f, 0.f);

    Engine::DebugPrintScene();

    while (Windows::MainLoop())
    {
        Core::BeginFrame();
        float dt = (float)Timer::GetDeltaTime();

        boneAngle += 60.f * dt;
        float bend = sinf(boneAngle * 0.01745329f) * 70.f * 0.01745329f;
        bones[1] = T0 * XMMatrixRotationZ(bend) * T1;

        Engine::SetEntityBoneMatrices(arm, bones, 2);
        Engine::TurnEntity(arm, 0.f, 25.f * dt, 0.f);
        Engine::TurnEntity(cube, 20.f * dt, 35.f * dt, 0.f);

        Engine::Cls(15, 20, 35);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();
        Core::EndFrame();
    }

    return 0;
}
