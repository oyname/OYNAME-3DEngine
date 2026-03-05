#pragma once
#include <cstdint>

// Render Layer Bitmasks
// Jedes Bit entspricht einem Layer. Meshes tragen einen layerMask,
// Kameras tragen eine cullMask. Nur wenn (mesh->layerMask & cam->cullMask) != 0
// wird das Mesh von dieser Kamera gerendert.

constexpr uint32_t LAYER_DEFAULT    = (1u << 0);  // Standard-Objekte
constexpr uint32_t LAYER_UI         = (1u << 1);  // UI / HUD
constexpr uint32_t LAYER_REFLECTION = (1u << 2);  // Reflektions-Pass
constexpr uint32_t LAYER_SHADOW     = (1u << 3);  // Shadow-Only Objekte
constexpr uint32_t LAYER_FX         = (1u << 4);  // Partikel / Effekte
constexpr uint32_t LAYER_5          = (1u << 5);
constexpr uint32_t LAYER_6          = (1u << 6);
constexpr uint32_t LAYER_7          = (1u << 7);
constexpr uint32_t LAYER_ALL        = 0xFFFFFFFFu; // Alles sichtbar
