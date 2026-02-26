#pragma once

// Small standalone header: GXFORMAT lives here so that high-traffic headers
// (like gdxdevice.h) don't have to include the heavy gdxutil.h / DirectX headers.

enum GXFORMAT {
    FORMAT_NONE = 0,
    B8G8R8A8_UNORM = 1 << 0,
    B8G8R8A8_UNORM_SRGB = 1 << 1,
    R8G8B8A8_UNORM = 1 << 2,
    R8G8B8A8_UNORM_SRGB = 1 << 3,
    R16G16B16A16_FLOAT = 1 << 4,
    R10G10B10A2_UNORM = 1 << 5,
    R10G10B10_XR_BIAS_A2_UNORM = 1 << 6,
};

// Bitwise ops for GXFORMAT (global)
inline GXFORMAT operator|(GXFORMAT a, GXFORMAT b)
{
    return static_cast<GXFORMAT>(static_cast<int>(a) | static_cast<int>(b));
}

inline GXFORMAT& operator|=(GXFORMAT& a, GXFORMAT b)
{
    a = a | b;
    return a;
}
