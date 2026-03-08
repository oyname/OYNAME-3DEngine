#include "geometry.h"
#include <cmath>

void CreateCube(LPENTITY* mesh, MATERIAL* material)
{
    LPSURFACE wuerfel = NULL;

    Engine::CreateMesh(mesh);
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

    if (material)
        Engine::SetSlotMaterial(*mesh, 0, material);
    Engine::FillBuffer(*mesh, 0u);
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
    Engine::SetSlotMaterial(*mesh, 0, matFront);
    Engine::FillBuffer(*mesh, 0);

    // ---- Vorne (+Z)  winding: (2,1,0) (2,3,1) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 0, 0, 1); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 2, 1, 0);
    Engine::AddTriangle(s, 2, 3, 1);
    Engine::SetSlotMaterial(*mesh, 1, matFront);
    Engine::FillBuffer(*mesh, 1);

    // ---- Links (-X)  winding: (0,1,2) (2,1,3) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, -1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 0, 1, 2);
    Engine::AddTriangle(s, 2, 1, 3);
    Engine::SetSlotMaterial(*mesh, 2, matSide);
    Engine::FillBuffer(*mesh, 2);

    // ---- Rechts (+X)  winding: (2,1,0) (2,3,1) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 1, 0, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 2, 1, 0);
    Engine::AddTriangle(s, 2, 3, 1);
    Engine::SetSlotMaterial(*mesh, 3, matSide);
    Engine::FillBuffer(*mesh, 3);

    // ---- Unten (-Y)  winding: (0,1,2) (2,1,3) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, -1, -1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, 1, -1, -1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, -1, -1, 1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, -1, 1); Engine::VertexNormal(s, 0, -1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 0, 1, 2);
    Engine::AddTriangle(s, 2, 1, 3);
    Engine::SetSlotMaterial(*mesh, 4, matTopBot);
    Engine::FillBuffer(*mesh, 4);

    // ---- Oben (+Y)  winding: (1,2,3) (2,1,0) ----
    Engine::CreateSurface(&s, *mesh);
    Engine::AddVertex(s, -1, 1, -1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 1.0f);
    Engine::AddVertex(s, 1, 1, -1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 1.0f);
    Engine::AddVertex(s, -1, 1, 1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 0.0f, 0.0f);
    Engine::AddVertex(s, 1, 1, 1); Engine::VertexNormal(s, 0, 1, 0); Engine::VertexColor(s, 224, 224, 224); Engine::VertexTexCoord(s, 1.0f, 0.0f);
    Engine::AddTriangle(s, 1, 2, 3);
    Engine::AddTriangle(s, 2, 1, 0);
    Engine::SetSlotMaterial(*mesh, 5, matTopBot);
    Engine::FillBuffer(*mesh, 5);
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

    Engine::FillBuffer(*mesh, 0u);
}
