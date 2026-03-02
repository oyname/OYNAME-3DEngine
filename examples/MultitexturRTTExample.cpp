#include "gidx.h"
#include "NeonTimeBuffer.h"

static void CreateCube(LPENTITY* mesh, MATERIAL* material);

// Zwei Wuerfel nebeneinander:
// Links  = Standard-Shader mit Textur
// Rechts = Neon-Plasma-Shader (animiert, zeitbasiert)

static NeonTimeBuffer g_neonTime;

int main()
{
    bool windowed = true;
    windowed == true ? Engine::Graphics(1200, 650) : Engine::Graphics(1980, 1080, false);

    // --- Neon-Shader erstellen ---
    // Nur POSITION wird benoetigt (keine Normals, keine UVs, kein Color)
    LPSHADER neonShader = nullptr;
    Engine::CreateShader(
        &neonShader,
        L"..\\shaders\\VertexShaderNeon.hlsl", "main",
        L"..\\shaders\\PixelShaderNeon.hlsl", "main",
        D3DVERTEX_POSITION
    );

    // NeonTimeBuffer initialisieren.
    // engine ist ueber gidx.h -> gdxengine.h bereits bekannt.
    if (!g_neonTime.Initialize(Engine::engine->m_device.GetDevice(), Engine::engine->m_device.GetDeviceContext()))
    {
        Debug::Log("game.cpp: NeonTimeBuffer-Initialisierung fehlgeschlagen.");
    }

    // --- Kamera ---
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 10.0f, 0.0f, 10.0f);
    Engine::LookAt(camera, -20.0f, 0.0f, 0.0f);

    // --- RTT-Kamera: filmt nur den linken Wuerfel ---
    LPENTITY rttCamera = nullptr;
    Engine::CreateCamera(&rttCamera);
    Engine::PositionEntity(rttCamera, 7.0f, 0.0f, 3.0f);
    Engine::LookAt(rttCamera, 2.0f, 0.0f, 0.0f);

    // --- Material 2: Neon-Plasma-Shader (rechter Wuerfel) ---
    LPMATERIAL neonMaterial = nullptr;
    Engine::CreateMaterial(&neonMaterial, neonShader);

    // ── RTT anlegen: 512×512 Render-Textur ───────────────────────────────────
    LPRENDERTARGET rtt = nullptr;
    Engine::CreateRenderTexture(&rtt, 512, 512);
    Engine::SetRTTClearColor(rtt, 0.0f, 0.25f, 0.5f); // dunkles Blau als RTT-Hintergrund

    // --- Linker Wuerfel: Standard-Shader ---
    // ── Material für den rechten Würfel: bekommt RTT-Textur ──────────────────
    LPMATERIAL materialRTT = nullptr;
    Engine::CreateMaterial(&materialRTT);
    Engine::MaterialTexture(materialRTT, Engine::GetRTTTexture(rtt));

    LPENTITY Mesh1 = nullptr;
    CreateCube(&Mesh1, materialRTT);
    Engine::PositionEntity(Mesh1, -10.0f, 0.0f, 13.0f);
    Engine::RotateEntity(Mesh1, 45.0f, 45.0f, 0.0f);
    Engine::ScaleEntity(Mesh1, 4.0f, 4.0f, 4.0f);

    // --- Rechter Wuerfel: Neon-Plasma-Shader ---
    LPENTITY Mesh2 = nullptr;
    CreateCube(&Mesh2, neonMaterial);
    Engine::PositionEntity(Mesh2, 2.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(Mesh2, 1.5f, 1.5f, 1.5f);

    // --- Licht ---
    LPENTITY directionalLight = nullptr;
    Engine::CreateLight(&directionalLight, D3DLIGHT_DIRECTIONAL);
    Engine::PositionEntity(directionalLight, 5.0f, 0.0f, -1.0f);
    Engine::LookAt(directionalLight, 2.0f, 0.0f, 0.0f);
    Engine::LightColor(directionalLight, 1.0f, 0.8f, 1.0f);
    Engine::SetDirectionalLight(directionalLight);

    Engine::SetVSync(1);

    // Render to Texture Kamera sieht Würfel nicht
    Engine::EntityLayer(Mesh1, LAYER_FX);
    Engine::CameraCullMask(rttCamera, LAYER_DEFAULT);

    // --- Textur fuer den linken Wuerfel ---
    LPTEXTURE face = nullptr;
    Engine::LoadTexture(&face, L"..\\media\\engine.png");
    LPMATERIAL materialFace = nullptr;
    Engine::CreateMaterial(&materialFace);
    Engine::MaterialTexture(materialFace, face);

    LPENTITY Mesh3 = nullptr;
    CreateCube(&Mesh3, materialFace);
    Engine::PositionEntity(Mesh3, -15.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(Mesh3, 1.0f, 20.0f, 20.0f);

    const float speed = 100.0f;

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();
        const float dt = static_cast<float>(Core::GetDeltaTime());

        // --- NeonTimeBuffer aktualisieren ---
        g_neonTime.Update(dt);

        //Engine::TurnEntity(Mesh1, -20.0f * dt, -20.0f * dt, -20.0f * dt);
        Engine::TurnEntity(Mesh2, 50.0f * dt, 50.0f * dt, 0.0f);

        // ── Pass 1: RTT-Pass ─────────────────────────────────────────────────
        
        Engine::EntityCastShadows(Mesh1, false);
        Engine::SetCamera(rttCamera);
        Engine::SetRenderTarget(rtt, rttCamera);
        Engine::UpdateWorld();
        Engine::RenderWorld();   // rendert in rtt (512x512)

        // ── Pass 2: Normaler Frame ───────────────────────────────────────────
        Engine::EntityCastShadows(Mesh1, true);
        Engine::ResetRenderTarget();
        Engine::SetCamera(camera);
        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        g_neonTime.Bind();
        Engine::RenderWorld();   // rendert in Backbuffer (1200x650)
        Engine::Flip();

        Core::EndFrame();
    }

    Engine::ReleaseRenderTexture(&rtt);
    g_neonTime.Shutdown();
    
    return 0;
}

void CreateCube(LPENTITY* mesh, MATERIAL* material)
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