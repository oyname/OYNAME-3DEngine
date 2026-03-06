#pragma once

#include "gidx.h"

void CreateCube(LPENTITY* mesh, MATERIAL* material = nullptr);

void CreateMultiMaterialCube(LPENTITY* mesh,
    LPMATERIAL matFront,    // Vorne/Hinten
    LPMATERIAL matSide,     // Links/Rechts
    LPMATERIAL matTopBot);  // Oben/Unten

void CreatePlate(LPENTITY* mesh);
