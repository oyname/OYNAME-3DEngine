// bone_animation_showcase.cpp
//
// Demonstriert Bone Animation (Skeletal Skinning) mit manuellem Mesh-Aufbau.
//
// Benoetigt im shaders/-Ordner:
//   SkinVertexShader.hlsl
//   SkinPixelShader.hlsl
//
// Das Mesh ist ein segmentiertes Rohr entlang der Y-Achse mit 6 Knochen.
// Eine Sinuswelle laeuft von unten nach oben und beugt das Rohr wie einen Tentakel.
//
// Knochen-Matrix-Berechnung (DirectX Zeilen-Vektor-Konvention):
//   boneWorld[0]  = RotX(a0) * RotZ(a0)
//   boneWorld[i]  = RotX(ai) * RotZ(ai) * Translation(0, SEG_H, 0) * boneWorld[i-1]
//   invBind[i]    = Translation(0, -i * SEG_H, 0)
//   finalMat[i]   = invBind[i] * boneWorld[i]
//
// finalMat[i] wird per SetEntityBoneMatrices an den Skinning-Vertex-Shader gesendet.

#include "gidx.h"
#include "geometry.h"
#include <cmath>

// ---- Mesh-Parameter -------------------------------------------------------
static const int   N_BONES = 6;
static const float SEG_H = 1.5f;
static const float HALF_W = 0.38f;
static const float HALF_D = 0.22f;
static const int   N_RINGS = 2 * N_BONES + 1;   // 13 Ringe
static const int   VERTS_RING = 4;

// ---------------------------------------------------------------------------
// BuildTailMesh
// ---------------------------------------------------------------------------
static void BuildTailMesh(LPENTITY* outMesh, LPMATERIAL mat)
{
    Engine::CreateMesh(outMesh);
    LPSURFACE surf = nullptr;
    Engine::CreateSurface(&surf, *outMesh);
    if (mat) Engine::SurfaceMaterial(surf, mat);

    // Eckpositionen und Normalen fuer einen Ring (4 Ecken)
    const float cx[VERTS_RING] = { -HALF_W,  HALF_W,  HALF_W, -HALF_W };
    const float cz[VERTS_RING] = { -HALF_D, -HALF_D,  HALF_D,  HALF_D };
    const float nx[VERTS_RING] = { -0.707f,  0.707f,  0.707f, -0.707f };
    const float nz[VERTS_RING] = { -0.707f, -0.707f,  0.707f,  0.707f };

    // ---- Vertices ---------------------------------------------------------
    for (int r = 0; r < N_RINGS; ++r)
    {
        const float t = r * 0.5f;
        const float y = t * SEG_H;
        const float gradient = y / (N_BONES * SEG_H);

        // Farbverlauf: dunkles Indigo -> helles Cyan
        const unsigned int cr = static_cast<unsigned int>(20 + 30 * gradient);
        const unsigned int cg = static_cast<unsigned int>(60 + 195 * gradient);
        const unsigned int cb = static_cast<unsigned int>(140 + 115 * gradient);

        for (int c = 0; c < VERTS_RING; ++c)
        {
            Engine::AddVertex(surf, cx[c], y, cz[c]);
            Engine::VertexNormal(surf, nx[c], 0.0f, nz[c]);
            Engine::VertexColor(surf, cr, cg, cb);
            Engine::VertexTexCoord(surf,
                static_cast<float>(c) / static_cast<float>(VERTS_RING - 1),
                gradient);
        }
    }

    // ---- Bone-Daten -------------------------------------------------------
    // Grenzringe (r gerade): 50/50 zwischen Knochen r/2-1 und r/2
    // Innenringe (r ungerade): vollstaendig Knochen r/2
    for (int r = 0; r < N_RINGS; ++r)
    {
        const int  boneBase = r / 2;
        const bool isBoundary = (r % 2 == 0);
        const int  v0 = r * VERTS_RING;

        for (int c = 0; c < VERTS_RING; ++c)
        {
            if (isBoundary)
            {
                if (boneBase == 0)
                    Engine::VertexBoneData(surf, v0 + c,
                        0, 0, 0, 0, 1.0f, 0.0f, 0.0f, 0.0f);
                else if (boneBase >= N_BONES)
                    Engine::VertexBoneData(surf, v0 + c,
                        N_BONES - 1, 0, 0, 0, 1.0f, 0.0f, 0.0f, 0.0f);
                else
                    Engine::VertexBoneData(surf, v0 + c,
                        boneBase - 1, boneBase, 0, 0, 0.5f, 0.5f, 0.0f, 0.0f);
            }
            else
            {
                Engine::VertexBoneData(surf, v0 + c,
                    boneBase, 0, 0, 0, 1.0f, 0.0f, 0.0f, 0.0f);
            }
        }
    }

    // ---- Dreiecke (doppelseitig) ------------------------------------------
    for (int r = 0; r < N_RINGS - 1; ++r)
    {
        const int b = r * VERTS_RING;
        const int t = (r + 1) * VERTS_RING;

        for (int f = 0; f < VERTS_RING; ++f)
        {
            const int nf = (f + 1) % VERTS_RING;
            Engine::AddTriangle(surf, b + f, t + f, b + nf);
            Engine::AddTriangle(surf, t + f, t + nf, b + nf);
            Engine::AddTriangle(surf, b + f, b + nf, t + f);
            Engine::AddTriangle(surf, t + f, b + nf, t + nf);
        }
    }

    Engine::FillBuffer(surf);

    Debug::Log("bone_animation_showcase.cpp: BuildTailMesh abgeschlossen - ",
        N_RINGS * VERTS_RING, " Vertices");
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
void main(LPVOID hwnd)
{
    Debug::Log("bone_animation_showcase.cpp: main() gestartet");
    Engine::Graphics(1280, 720, true);

    // ---- Skinning-Shader laden -------------------------------------------
    // SkinVertexShader.hlsl und SkinPixelShader.hlsl muessen im shaders/-Ordner liegen.
    // Vertex Flags muessen BONE_INDICES und BONE_WEIGHTS enthalten damit FillBuffer
    // die Bone-Daten in GPU-Buffer schreibt.
    LPSHADER skinShader = nullptr;
    DWORD skinFlags = D3DVERTEX_POSITION | D3DVERTEX_NORMAL |
        D3DVERTEX_COLOR | D3DVERTEX_TEX1 |
        D3DVERTEX_BONE_INDICES | D3DVERTEX_BONE_WEIGHTS;

    HRESULT hr = Engine::CreateShader(
        &skinShader,
        L"..\\shaders\\SkinVertexShader.hlsl", "main",
        L"..\\shaders\\PixelShader.hlsl", "main",
        skinFlags);

    if (FAILED(hr))
    {
        Debug::Log("bone_animation_showcase.cpp: FEHLER - Skinning-Shader konnte nicht geladen werden");
        return;
    }
    Debug::Log("bone_animation_showcase.cpp: Skinning-Shader geladen");

    // ---- Kamera ----------------------------------------------------------
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 5.0f, -12.0f);
    Engine::LookAt(camera, 0.0f, 4.5f, 0.0f);

    // ---- Licht -----------------------------------------------------------
    LPENTITY sunLight = nullptr;
    Engine::CreateLight(&sunLight, D3DLIGHT_DIRECTIONAL);
    Engine::LightColor(sunLight, 1.0f, 0.95f, 0.85f);
    Engine::PositionEntity(sunLight, 5.0f, 12.0f, -5.0f);
    Engine::LookAt(sunLight, 0.0f, 4.0f, 0.0f);
    Engine::SetDirectionalLight(sunLight);
    Engine::LightShadowOrthoSize(sunLight, 16.0f);
    Engine::LightShadowPlanes(sunLight, 0.5f, 30.0f);
    Engine::SetAmbientColor(0.12f, 0.15f, 0.22f);

    // ---- Boden -----------------------------------------------------------
    LPMATERIAL groundMat = nullptr;
    Engine::CreateMaterial(&groundMat);
    Engine::MaterialColor(groundMat, 0.18f, 0.18f, 0.26f, 1.0f);
    Engine::MaterialUsePBR(groundMat, true);
    Engine::MaterialMetallic(groundMat, 0.0f);
    Engine::MaterialRoughness(groundMat, 0.85f);
    Engine::MaterialReceiveShadows(groundMat, true);

    LPENTITY ground = nullptr;
    CreateCube(&ground, groundMat);
    Engine::EntityCastShadows(ground, false);
    Engine::PositionEntity(ground, 0.0f, -0.5f, 0.0f);
    Engine::ScaleEntity(ground, 12.0f, 0.4f, 12.0f);

    // ---- Skinning-Material ----------------------------------------------
    // CreateMaterial mit explizitem skinShader -- Material wird an diesen Shader gebunden.
    // Dadurch bekommt FillBuffer den richtigen flagsVertex-Wert mit BONE_INDICES/WEIGHTS.
    LPMATERIAL tailMat = nullptr;
    Engine::CreateMaterial(&tailMat, skinShader);
    Engine::MaterialColor(tailMat, 1.0f, 1.0f, 1.0f, 1.0f);

    // ---- Skinned Mesh aufbauen ------------------------------------------
    LPENTITY tailMesh = nullptr;
    BuildTailMesh(&tailMesh, tailMat);
    Engine::PositionEntity(tailMesh, 0.0f, 0.0f, 0.0f);

    // InvBind[i] = Inverse der Bindpose-Welttransformation von Knochen i.
    // Bindpose: boneWorld[i] = Translation(0, SEG_H, 0)^i = Translation(0, i*SEG_H, 0)
    // Inverse = Translation(0, -i*SEG_H, 0)
    DirectX::XMMATRIX invBind[N_BONES];
    for (int i = 0; i < N_BONES; ++i)
        invBind[i] = DirectX::XMMatrixTranslation(
            0.0f, -static_cast<float>(i) * SEG_H, 0.0f);

    Debug::Log("bone_animation_showcase.cpp: Szene aufgebaut");

    // ---- Hauptschleife ---------------------------------------------------
    float timeAcc = 0.0f;
    float camAngle = 0.0f;

    const float WAVE_AMP = 0.30f;
    const float WAVE_FREQ = 1.8f;
    const float PHASE_STEP = DirectX::XM_PI / static_cast<float>(N_BONES);
    const float WAVE_AMP_Z = 0.20f;
    const float WAVE_FREQ_Z = 1.3f;

    while (Windows::MainLoop())
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Timer::GetDeltaTime());
        timeAcc += dt;
        camAngle += 6.0f * dt;

        // Kamera kreist langsam
        const float camRad = camAngle * DirectX::XM_PI / 180.0f;
        Engine::PositionEntity(camera,
            10.0f * std::sin(camRad), 5.0f,
            -10.0f * std::cos(camRad));
        Engine::LookAt(camera, 0.0f, 4.5f, 0.0f);

        // ---- Bone-Matrizen berechnen -------------------------------------
        DirectX::XMMATRIX boneWorld[N_BONES];
        DirectX::XMMATRIX finalMats[N_BONES];

        // Knochen 0: dreht um Wurzel (Y=0)
        const float ax0 = WAVE_AMP * std::sin(timeAcc * WAVE_FREQ);
        const float az0 = WAVE_AMP_Z * std::sin(timeAcc * WAVE_FREQ_Z
            + DirectX::XM_PIDIV2);
        boneWorld[0] = DirectX::XMMatrixRotationX(ax0)
            * DirectX::XMMatrixRotationZ(az0);

        // Knochen i:
        // Reihenfolge (row-vector): erst zur Elternspitze translieren,
        // dann lokal rotieren, dann Elternkette anwenden.
        // Translation vor Rotation = Pivot liegt an der Elternspitze.
        for (int i = 1; i < N_BONES; ++i)
        {
            const float phi = i * PHASE_STEP;
            const float axi = WAVE_AMP * std::sin(timeAcc * WAVE_FREQ + phi);
            const float azi = WAVE_AMP_Z * std::sin(timeAcc * WAVE_FREQ_Z
                + phi + DirectX::XM_PIDIV2);

            boneWorld[i] = DirectX::XMMatrixTranslation(0.0f, SEG_H, 0.0f)
                * DirectX::XMMatrixRotationX(axi)
                * DirectX::XMMatrixRotationZ(azi)
                * boneWorld[i - 1];
        }

        for (int i = 0; i < N_BONES; ++i)
            finalMats[i] = invBind[i] * boneWorld[i];

        Engine::SetEntityBoneMatrices(tailMesh, finalMats, N_BONES);

        Engine::Cls(5, 8, 16);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    Debug::Log("bone_animation_showcase.cpp: main() beendet");
}