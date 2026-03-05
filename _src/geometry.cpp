#include "geometry.h"

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


    // Back (verts 0-3)
    Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f);
    // Front (verts 4-7)
    Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f);
    // Left (verts 8-11)
    Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f);
    Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f);
    // Right (verts 12-15)
    Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f);
    Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f);
    // Bottom (verts 16-19)
    Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f);
    Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f);
    // Top (verts 20-23)
    Engine::VertexTexCoord2(wuerfel, 1.0f, 0.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 0.0f);
    Engine::VertexTexCoord2(wuerfel, 1.0f, 1.0f); Engine::VertexTexCoord2(wuerfel, 0.0f, 1.0f);

    Engine::AddTriangle(wuerfel, 0, 1, 2); Engine::AddTriangle(wuerfel, 3, 2, 1);
    Engine::AddTriangle(wuerfel, 6, 5, 4); Engine::AddTriangle(wuerfel, 6, 7, 5);
    Engine::AddTriangle(wuerfel, 8, 9, 10); Engine::AddTriangle(wuerfel, 10, 9, 11);
    Engine::AddTriangle(wuerfel, 14, 13, 12); Engine::AddTriangle(wuerfel, 14, 15, 13);
    Engine::AddTriangle(wuerfel, 16, 17, 18); Engine::AddTriangle(wuerfel, 18, 17, 19);
    Engine::AddTriangle(wuerfel, 21, 22, 23); Engine::AddTriangle(wuerfel, 22, 21, 20);

    Engine::FillBuffer(wuerfel);
}

void CreateMultiMaterialCube(LPENTITY* mesh,
    LPMATERIAL matFront,   // Vorne/Hinten
    LPMATERIAL matSide,    // Links/Rechts
    LPMATERIAL matTopBot)  // Oben/Unten
{
    Engine::CreateMesh(mesh);

    LPSURFACE s = nullptr;

    // ---- Hinten (-Z)  winding: (0,1,2) (3,2,1) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 0, 0, -1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 0, 1, 2);
    Engine::AddTriangle(s, 3, 2, 1);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matFront);

    // ---- Vorne (+Z)  winding: (2,1,0) (2,3,1) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 2, 1, 0);
    Engine::AddTriangle(s, 2, 3, 1);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matFront);

    // ---- Links (-X)  winding: (0,1,2) (2,1,3) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 0, 1, 2);
    Engine::AddTriangle(s, 2, 1, 3);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matSide);

    // ---- Rechts (+X)  winding: (2,1,0) (2,3,1) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 2, 1, 0);
    Engine::AddTriangle(s, 2, 3, 1);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matSide);

    // ---- Unten (-Y)  winding: (0,1,2) (2,1,3) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 0, 1, 2);
    Engine::AddTriangle(s, 2, 1, 3);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matTopBot);

    // ---- Oben (+Y)  winding: (1,2,3) (2,1,0) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 1, 2, 3);
    Engine::AddTriangle(s, 2, 1, 0);
    Engine::FillBuffer(s);
    Engine::SurfaceMaterial(s, matTopBot);
}

void CreatePlate(LPENTITY *mesh)
{
    LPSURFACE surface = nullptr;
    Engine::CreateSurface(&surface, (*mesh));

    // Quad in X/Z plane (Y = 0)
    Engine::AddVertex(surface, -1, 0, -1);
    Engine::VertexNormal(surface, 0, 1, 0);
    Engine::VertexColor(surface, 200, 200, 200);
    Engine::VertexTexCoord(surface, 0.0f, 1.0f);

    Engine::AddVertex(surface, 1, 0, -1);
    Engine::VertexNormal(surface, 0, 1, 0);
    Engine::VertexColor(surface, 200, 200, 200);
    Engine::VertexTexCoord(surface, 1.0f, 1.0f);

    Engine::AddVertex(surface, -1, 0, 1);
    Engine::VertexNormal(surface, 0, 1, 0);
    Engine::VertexColor(surface, 200, 200, 200);
    Engine::VertexTexCoord(surface, 0.0f, 0.0f);

    Engine::AddVertex(surface, 1, 0, 1);
    Engine::VertexNormal(surface, 0, 1, 0);
    Engine::VertexColor(surface, 200, 200, 200);
    Engine::VertexTexCoord(surface, 1.0f, 0.0f);

    // Two triangles (watch winding order if culling looks wrong)
    Engine::AddTriangle(surface, 1, 2, 3);
    Engine::AddTriangle(surface, 2, 1, 0);

    Engine::FillBuffer(surface);
}

void CreateSimpleCube(LPENTITY* mesh, MATERIAL* material)
{
    Engine::CreateMesh(mesh, material);

    LPSURFACE surface = nullptr;
    Engine::CreateSurface(&surface, *mesh);

    // 8 Eckpunkte des Würfels
    float size = 1.0f;
    DirectX::XMFLOAT3 vertices[8] = {
        {-size, -size, -size},  // 0
        {-size, +size, -size},  // 1
        {+size, +size, -size},  // 2
        {+size, -size, -size},  // 3
        {-size, -size, +size},  // 4
        {-size, +size, +size},  // 5
        {+size, +size, +size},  // 6
        {+size, -size, +size}   // 7
    };

    // 6 Seiten, 4 Vertices pro Seite = 24 Vertices
    // Front Face (Z-)
    Engine::AddVertex(surface, vertices[0]); Engine::VertexNormal(surface, 0, 0, -1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[1]); Engine::VertexNormal(surface, 0, 0, -1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[2]); Engine::VertexNormal(surface, 0, 0, -1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[3]); Engine::VertexNormal(surface, 0, 0, -1); Engine::VertexColor(surface, 255, 255, 255);

    // Back Face (Z+)  <-- Winding gedreht
    Engine::AddVertex(surface, vertices[4]); Engine::VertexNormal(surface, 0, 0, 1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[7]); Engine::VertexNormal(surface, 0, 0, 1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[6]); Engine::VertexNormal(surface, 0, 0, 1); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[5]); Engine::VertexNormal(surface, 0, 0, 1); Engine::VertexColor(surface, 255, 255, 255);

    // Left Face (X-)
    Engine::AddVertex(surface, vertices[0]); Engine::VertexNormal(surface, -1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[4]); Engine::VertexNormal(surface, -1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[5]); Engine::VertexNormal(surface, -1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[1]); Engine::VertexNormal(surface, -1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);

    // Right Face (X+)
    Engine::AddVertex(surface, vertices[3]); Engine::VertexNormal(surface, 1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[2]); Engine::VertexNormal(surface, 1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[6]); Engine::VertexNormal(surface, 1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[7]); Engine::VertexNormal(surface, 1, 0, 0); Engine::VertexColor(surface, 255, 255, 255);

    // Bottom Face (Y-)
    Engine::AddVertex(surface, vertices[0]); Engine::VertexNormal(surface, 0, -1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[3]); Engine::VertexNormal(surface, 0, -1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[7]); Engine::VertexNormal(surface, 0, -1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[4]); Engine::VertexNormal(surface, 0, -1, 0); Engine::VertexColor(surface, 255, 255, 255);

    // Top Face (Y+)
    Engine::AddVertex(surface, vertices[1]); Engine::VertexNormal(surface, 0, 1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[5]); Engine::VertexNormal(surface, 0, 1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[6]); Engine::VertexNormal(surface, 0, 1, 0); Engine::VertexColor(surface, 255, 255, 255);
    Engine::AddVertex(surface, vertices[2]); Engine::VertexNormal(surface, 0, 1, 0); Engine::VertexColor(surface, 255, 255, 255);

    // Indices für alle 6 Seiten (2 Triangles pro Seite)
    int offset = 0;
    for (int face = 0; face < 6; face++) {
        Engine::AddTriangle(surface, offset + 0, offset + 1, offset + 2);
        Engine::AddTriangle(surface, offset + 0, offset + 2, offset + 3);
        offset += 4;
    }

    Engine::FillBuffer(surface);
}