#include "gdxutil.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace
{
    struct FormatName { GXFORMAT fmt; const wchar_t* name; };

    constexpr FormatName kFormatNames[] =
    {
        { FORMAT_NONE,                 L"FORMAT_NONE" },
        { B8G8R8A8_UNORM,              L"B8G8R8A8_UNORM" },
        { B8G8R8A8_UNORM_SRGB,         L"B8G8R8A8_UNORM_SRGB" },
        { R8G8B8A8_UNORM,              L"R8G8B8A8_UNORM" },
        { R8G8B8A8_UNORM_SRGB,         L"R8G8B8A8_UNORM_SRGB" },
        { R16G16B16A16_FLOAT,          L"R16G16B16A16_FLOAT" },
        { R10G10B10A2_UNORM,           L"R10G10B10A2_UNORM" },
        { R10G10B10_XR_BIAS_A2_UNORM,  L"R10G10B10_XR_BIAS_A2_UNORM" },
    };
}

namespace GXUTIL
{
    DXGI_FORMAT GetDXGIFormat(GXFORMAT format)
    {
        switch (format)
        {
        case B8G8R8A8_UNORM:               return DXGI_FORMAT_B8G8R8A8_UNORM;
        case B8G8R8A8_UNORM_SRGB:          return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case R8G8B8A8_UNORM:               return DXGI_FORMAT_R8G8B8A8_UNORM;
        case R8G8B8A8_UNORM_SRGB:          return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case R16G16B16A16_FLOAT:           return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case R10G10B10A2_UNORM:            return DXGI_FORMAT_R10G10B10A2_UNORM;
        case R10G10B10_XR_BIAS_A2_UNORM:   return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
        default:                           return DXGI_FORMAT_UNKNOWN;
        }
    }

    std::wstring GetTextureFormatName(GXFORMAT format)
    {
        for (const auto& it : kFormatNames)
            if (it.fmt == format)
                return it.name;
        return L"Unknown";
    }

    bool isHardwareSupported(IDXGIAdapter* pAdapter)
    {
        if (!pAdapter) return false;

        DXGI_ADAPTER_DESC desc{};
        if (FAILED(pAdapter->GetDesc(&desc)))
            return false;

        return desc.DedicatedVideoMemory > 0;
    }

    bool isWindowsRenderer(IDXGIAdapter* adapter)
    {
        if (!adapter) return false;

        DXGI_ADAPTER_DESC desc{};
        if (FAILED(adapter->GetDesc(&desc)))
            return false;

        // Microsoft Basic Render Driver (WARP)
        return (desc.VendorId == 0x1414) && (desc.DeviceId == 0x008C);
    }

    GXFORMAT GetSupportedFormats(D3D_FEATURE_LEVEL featureLevel)
    {
        GXFORMAT supported = FORMAT_NONE;

        switch (featureLevel)
        {
        case D3D_FEATURE_LEVEL_9_1:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_3:
            supported |= B8G8R8A8_UNORM | B8G8R8A8_UNORM_SRGB |
                R8G8B8A8_UNORM | R8G8B8A8_UNORM_SRGB;
            break;

        case D3D_FEATURE_LEVEL_10_0:
        case D3D_FEATURE_LEVEL_10_1:
        case D3D_FEATURE_LEVEL_11_0:
        case D3D_FEATURE_LEVEL_11_1:
        case D3D_FEATURE_LEVEL_12_0:
        case D3D_FEATURE_LEVEL_12_1:
            supported |= B8G8R8A8_UNORM | B8G8R8A8_UNORM_SRGB |
                R8G8B8A8_UNORM | R8G8B8A8_UNORM_SRGB |
                R16G16B16A16_FLOAT |
                R10G10B10A2_UNORM |
                R10G10B10_XR_BIAS_A2_UNORM;
            break;

        default:
            break;
        }

        return supported;
    }

    //
    int GetFeatureLevel(D3D_FEATURE_LEVEL featureLevel)
    {
        // ZUSÄTZLICHER FIX: Falls noch Byte-Swap-Fehler durchkommen
        if (featureLevel == 0xC100) featureLevel = D3D_FEATURE_LEVEL_12_1;
        if (featureLevel == 0xC000) featureLevel = D3D_FEATURE_LEVEL_12_0;
        if (featureLevel == 0xB100) featureLevel = D3D_FEATURE_LEVEL_11_1;
        if (featureLevel == 0xB000) featureLevel = D3D_FEATURE_LEVEL_11_0;
        if (featureLevel == 0xA100) featureLevel = D3D_FEATURE_LEVEL_10_1;
        if (featureLevel == 0xA000) featureLevel = D3D_FEATURE_LEVEL_10_0;

        switch (featureLevel)
        {
        case D3D_FEATURE_LEVEL_9_1:
        case D3D_FEATURE_LEVEL_9_2:
        case D3D_FEATURE_LEVEL_9_3: return 1; // DX9

        case D3D_FEATURE_LEVEL_10_0: return 2; // DX10
        case D3D_FEATURE_LEVEL_10_1: return 3; // DX10.1

        case D3D_FEATURE_LEVEL_11_0: return 4; // DX11
        case D3D_FEATURE_LEVEL_11_1: return 5; // DX11.1

        case D3D_FEATURE_LEVEL_12_0: return 6; // DX12
        case D3D_FEATURE_LEVEL_12_1: return 7; // DX12.1

        default: return 0;
        }
    }

    D3D_FEATURE_LEVEL GetFeatureLevelFromDirectXVersion(int version)
    {
        switch (version)
        {
        case 1: return D3D_FEATURE_LEVEL_9_1;
        case 2: return D3D_FEATURE_LEVEL_10_0;
        case 3: return D3D_FEATURE_LEVEL_10_1;
        case 4: return D3D_FEATURE_LEVEL_11_0;
        case 5: return D3D_FEATURE_LEVEL_11_1;
        case 6: return D3D_FEATURE_LEVEL_12_0;
        case 7: return D3D_FEATURE_LEVEL_12_1;
        default: return D3D_FEATURE_LEVEL_9_1;
        }
    }

    std::wstring GetFeatureLevelName(D3D_FEATURE_LEVEL featureLevel)
    {
        switch (featureLevel)
        {
        case D3D_FEATURE_LEVEL_9_1:  return L"Direct3D 9.1";
        case D3D_FEATURE_LEVEL_9_2:  return L"Direct3D 9.2";
        case D3D_FEATURE_LEVEL_9_3:  return L"Direct3D 9.3";
        case D3D_FEATURE_LEVEL_10_0: return L"Direct3D 10.0";
        case D3D_FEATURE_LEVEL_10_1: return L"Direct3D 10.1";
        case D3D_FEATURE_LEVEL_11_0: return L"Direct3D 11.0";
        case D3D_FEATURE_LEVEL_11_1: return L"Direct3D 11.1";
        case D3D_FEATURE_LEVEL_12_0: return L"Direct3D 12.0";
        case D3D_FEATURE_LEVEL_12_1: return L"Direct3D 12.1";
        default: return L"Unknown";
        }
    }
} // namespace GXUTIL
