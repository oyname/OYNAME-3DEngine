#include "gidx.h"
#include "geometry.h"
#include "NeonTimeBuffer.h"

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

    // --- Textur fuer den linken Wuerfel ---
    LPTEXTURE face = nullptr;
    Engine::LoadTexture(&face, L"..\\media\\engine.png");

    // --- Kamera ---
    LPENTITY camera = nullptr;
    Engine::CreateCamera(&camera);
    Engine::PositionEntity(camera, 0.0f, 0.0f, -5.0f);
    Engine::RotateEntity(camera, 0.0f, 0.0f, 0.0f);

    // --- Material 1: Standard-Shader mit Textur (linker Wuerfel) ---
    LPMATERIAL material1 = nullptr;
    Engine::CreateMaterial(&material1);
    Engine::MaterialTexture(material1, face);

    // --- Material 2: Neon-Plasma-Shader (rechter Wuerfel) ---
    LPMATERIAL material2 = nullptr;
    Engine::CreateMaterial(&material2, neonShader);

    // --- Linker Wuerfel: Standard-Shader ---
    LPENTITY Mesh1 = nullptr;
    CreateCube(&Mesh1, material1);
    Engine::PositionEntity(Mesh1, -2.0f, 0.0f, 0.0f);
    Engine::ScaleEntity(Mesh1, 1.0f, 1.0f, 1.0f);

    // --- Rechter Wuerfel: Neon-Plasma-Shader ---
    LPENTITY Mesh2 = nullptr;
    CreateCube(&Mesh2, material2);
    Engine::PositionEntity(Mesh2, 2.0f, 0.0f, 0.0f);

    // --- Licht ---
    LPENTITY directionalLight = nullptr;
    Engine::CreateLight(&directionalLight, D3DLIGHT_POINT);
    Engine::PositionEntity(directionalLight, 0.0f, 0.0f, -10.0f);
    Engine::RotateEntity(directionalLight, 90.0f, 20.0f, 0.0f);
    Engine::LightColor(directionalLight, 1.0f, 1.0f, 1.0f);

    Engine::SetVSync(1);

    const float speed = 100.0f;

    Engine::DebugPrintScene();

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Core::BeginFrame();

        const float dt = static_cast<float>(Timer::GetDeltaTime());

        // --- Steuerung linker Wuerfel ---
        if (GetAsyncKeyState(VK_UP) & 0x8000) Engine::MoveEntity(Mesh1, 0.0f, 0.0f, 5.0f * dt);
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) Engine::MoveEntity(Mesh1, 0.0f, 0.0f, -5.0f * dt);
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) Engine::MoveEntity(Mesh1, 5.0f * dt, 0.0f, 0.0f);
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) Engine::MoveEntity(Mesh1, -5.0f * dt, 0.0f, 0.0f);

        // --- NeonTimeBuffer aktualisieren ---
        // Zeit hochzaehlen und den DYNAMIC Buffer per Map/Unmap auf die GPU schreiben
        g_neonTime.Update(dt);

        Engine::TurnEntity(Mesh1, 1.0f * -50 * dt, 1.0f * -50 * dt, 0.0f);
        Engine::TurnEntity(Mesh2, 1.0f * 50 * dt, 1.0f * 50 * dt, 0.0f);

        // --- Rendering ---
        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();

        // Buffer an PS-Register b1 binden, bevor die Scene gerendert wird.
        // b0 = MatrixBuffer (von der Engine selbst gesetzt, unveraendert)
        // b1 = TimeBuffer   (unser Neon-Effekt)
        g_neonTime.Bind();

        Engine::RenderWorld();
        Engine::Flip();

        Core::EndFrame();
    }

    g_neonTime.Shutdown();
    return 0;
}