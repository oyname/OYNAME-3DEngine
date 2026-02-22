// MultitexturExample.cpp
// Demonstriert: Zwei Texturen mit einstellbarem Blend-Modus.
//
// Blend-Modi:
//   0 = off          nur Textur 1
//   1 = Multiply     Lightmap, Schatten
//   2 = Multiply×2   Detail-Map
//   3 = Additive     Glühen, Feuer
//   4 = Lerp(Alpha)  Decals, Aufkleber
//   5 = Luminanz     Overlay mit schwarzem Hintergrund  ← für texFace

#include <Windows.h>
#include "gidx.h"

LPENTITY g_quadMesh = nullptr;
LPENTITY g_camera = nullptr;

int main()
{
    Engine::Graphics(1200, 650);

    LPTEXTURE texBrick = nullptr;
    Engine::LoadTexture(&texBrick, L"..\\media\\dx.bmp");

    LPTEXTURE texFace = nullptr;
    Engine::LoadTexture(&texFace, L"..\\media\\face.bmp");

    Engine::CreateCamera(&g_camera);
    Engine::PositionEntity(g_camera, 0.0f, 0.0f, -5.0f);

    // Shader-Pfade genauso auflösen wie der Standard-Shader in gdxengine.cpp
    std::wstring vs = Core::ResolvePath(L"..\\..\\shaders\\VertexShader.hlsl");
    std::wstring ps = Core::ResolvePath(L"..\\..\\shaders\\PixelShader.hlsl");

    // Eigener Shader mit TEX1 | TEX2
    LPSHADER multiTexShader = nullptr;
    Engine::CreateShader(&multiTexShader,
        vs, "main",
        ps, "main",
        D3DVERTEX_POSITION | D3DVERTEX_NORMAL | D3DVERTEX_COLOR | D3DVERTEX_TEX1 | D3DVERTEX_TEX2);

    // Material: zwei Texturen + Blend-Modus 5 (Luminanz)
    LPMATERIAL material = nullptr;
    Engine::CreateMaterial(&material, multiTexShader);

    Engine::MaterialTexture(material, texBrick, 0);  // t0
    Engine::MaterialTexture(material, texFace, 1);   // t1
    Engine::MaterialBlendMode(material, 1);          // 0=off 1=multiply 2=mul×2 3=add 4=lerp 5=luminanz

    Engine::MaterialColor(material, 1.0f, 1.0f, 1.0f);    // Diffuse-Farbe
    Engine::MaterialShininess(material, 100.0f);            // Specular-Schärfe
    Engine::MaterialBlendMode(material, 1);      // multiply
    Engine::MaterialBlendFactor(material, 0.0f); // 35% Einfluss der 2. Textur

    LPENTITY light = nullptr;
    Engine::CreateLight(&light, D3DLIGHT_DIRECTIONAL);
    Engine::LightColor(light, 1.0f, 1.0f, 1.0f);

    Engine::CreateMesh(&g_quadMesh, material);

    LPSURFACE quad = nullptr;
    Engine::CreateSurface(&quad, g_quadMesh);

    Engine::AddVertex(quad, -1.0f, -1.0f, 0.0f); Engine::VertexNormal(quad, 0, 0, -1); Engine::VertexColor(quad, 255, 255, 255);
    Engine::AddVertex(quad, -1.0f, 1.0f, 0.0f); Engine::VertexNormal(quad, 0, 0, -1); Engine::VertexColor(quad, 255, 255, 255);
    Engine::AddVertex(quad, 1.0f, -1.0f, 0.0f); Engine::VertexNormal(quad, 0, 0, -1); Engine::VertexColor(quad, 255, 255, 255);
    Engine::AddVertex(quad, 1.0f, 1.0f, 0.0f); Engine::VertexNormal(quad, 0, 0, -1); Engine::VertexColor(quad, 255, 255, 255);

    // Slot 0: Albedo UV
    Engine::VertexTexCoord(quad, 0.0f, 1.0f);
    Engine::VertexTexCoord(quad, 0.0f, 0.0f);
    Engine::VertexTexCoord(quad, 1.0f, 1.0f);
    Engine::VertexTexCoord(quad, 1.0f, 0.0f);

    // Slot 1: Lightmap / Detail UV
    Engine::VertexTexCoord2(quad, 0.0f, 1.0f);
    Engine::VertexTexCoord2(quad, 0.0f, 0.0f);
    Engine::VertexTexCoord2(quad, 1.0f, 1.0f);
    Engine::VertexTexCoord2(quad, 1.0f, 0.0f);

    Engine::AddTriangle(quad, 0, 1, 2);
    Engine::AddTriangle(quad, 3, 2, 1);
    Engine::FillBuffer(quad);

    while (Windows::MainLoop() && !(GetAsyncKeyState(VK_ESCAPE) & 0x8000))
    {
        Engine::Cls(0, 64, 128);
        Engine::UpdateWorld();
        Engine::RenderWorld();
        Engine::Flip();
    }

    return 0;
}