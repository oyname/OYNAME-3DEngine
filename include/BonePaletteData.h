#pragma once

#include <DirectXMath.h>

// Maximale Anzahl Bones pro Mesh im Skinning-Shader
static const int MAX_BONES = 128;

// Engine-side Bone-Palette-Daten fuer Skinning.
// Der eigentliche DX11-Constant-Buffer wird im Backend verwaltet.
__declspec(align(16))
struct BonePaletteData
{
    DirectX::XMMATRIX boneMatrices[MAX_BONES];
};

// Legacy alias. Schrittweise auf BonePaletteData umstellen.
using BoneBuffer = BonePaletteData;
