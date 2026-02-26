#pragma once

// API-neutrales Viewport-Struct.
// Wird im Backend (DX11/Vulkan/...) in die jeweilige API-Struktur konvertiert.
struct Viewport
{
    float x        = 0.0f;
    float y        = 0.0f;
    float width    = 0.0f;
    float height   = 0.0f;
    float minDepth = 0.0f;
    float maxDepth = 1.0f;
};
