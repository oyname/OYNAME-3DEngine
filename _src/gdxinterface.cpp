// gdxinterface.cpp
#include "gdxinterface.h"


GDXInterface::GDXInterface() : m_bInitialized(false), m_factory(nullptr)
{
}

GDXInterface::~GDXInterface()
{
    Release();
}

HRESULT GDXInterface::Init(unsigned int bpp)
{
    (void)bpp; // Original: bpp unbenutzt

    Release();

    HRESULT hr = S_OK;

    interfaceManager.CleanupResources();
    interfaceManager.SetDXGI_Format(DXGI_FORMAT_R8G8B8A8_UNORM);

    // Create Factory
    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&m_factory);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    // Enumerate system information
    hr = GetSystemInfo();

    if (SUCCEEDED(hr))
    {
        m_bInitialized = true;
    }

    return hr;
}

void GDXInterface::Release()
{
    Memory::SafeRelease(m_factory);
    m_bInitialized = false;
}

HRESULT GDXInterface::GetSystemInfo()
{
    if (!m_factory)
        return E_FAIL;

    HRESULT hr = S_OK;
    IDXGIAdapter* adapter = nullptr;

    for (UINT i = 0; m_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        if (!adapter)
            continue;

        // Skip Windows software renderer
        if (GXUTIL::isWindowsRenderer(adapter))
        {
            Memory::SafeRelease(adapter);
            continue;
        }

        // Check hardware support
        bool hardwareSupported = GXUTIL::isHardwareSupported(adapter);
        interfaceManager.SetHardwareGPU(hardwareSupported);

        GXADAPTER gxAdapter;

        hr = adapter->GetDesc(&gxAdapter.Desc);
        if (FAILED(hr))
        {
            Debug::LogHr(__FILE__, __LINE__, hr);
            Memory::SafeRelease(adapter);
            return hr;
        }

        hr = EnumerateAdapterOutputs(adapter, gxAdapter);
        if (FAILED(hr))
        {
            Debug::LogHr(__FILE__, __LINE__, hr);
            Memory::SafeRelease(adapter);
            return hr;
        }

        interfaceManager.AddAdapter(gxAdapter);

        Memory::SafeRelease(adapter);
    }

    return hr;
}

HRESULT GDXInterface::EnumerateAdapterOutputs(IDXGIAdapter* adapter, GXADAPTER& gxAdapter)
{
    if (!adapter)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    IDXGIOutput* output = nullptr;

    for (UINT j = 0; adapter->EnumOutputs(j, &output) != DXGI_ERROR_NOT_FOUND; ++j)
    {
        if (!output)
            continue;

        GXOUTPUT gxOutput;

        hr = output->GetDesc(&gxOutput.OutputDesc);
        if (FAILED(hr))
        {
            Debug::LogHr(__FILE__, __LINE__, hr);
            Memory::SafeRelease(output);
            return hr;
        }

        hr = EnumerateDisplayModes(output, gxOutput);
        if (FAILED(hr))
        {
            Memory::SafeRelease(output);
            return hr;
        }

        gxAdapter.Outputs.push_back(gxOutput);

        Memory::SafeRelease(output);
    }

    return hr;
}

HRESULT GDXInterface::EnumerateDisplayModes(IDXGIOutput* output, GXOUTPUT& gxOutput)
{
    if (!output)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    DXGI_FORMAT format = interfaceManager.GetDXGI_Format();

    UINT numModes = 0;
    hr = output->GetDisplayModeList(format, 0, &numModes, nullptr);

    if (FAILED(hr) || numModes == 0)
        return hr;

    std::vector<DXGI_MODE_DESC> displayModes(numModes);
    hr = output->GetDisplayModeList(format, 0, &numModes, displayModes.data());

    if (FAILED(hr))
        return hr;

    for (const auto& mode : displayModes)
    {
        GXDISPLAYMODE gxMode;
        gxMode.Width = mode.Width;
        gxMode.Height = mode.Height;

        auto resolutionIt = std::find_if(
            gxOutput.DisplayModes.begin(),
            gxOutput.DisplayModes.end(),
            [&gxMode](const GXDISPLAYMODE& dm) {
                return dm.Width == gxMode.Width && dm.Height == gxMode.Height;
            }
        );

        float frequency = mode.RefreshRate.Denominator != 0
            ? static_cast<float>(mode.RefreshRate.Numerator) / mode.RefreshRate.Denominator
            : 0.0f;

        GXFREQUENCY gxFrequency;
        gxFrequency.Denominator = mode.RefreshRate.Denominator;
        gxFrequency.Numerator = mode.RefreshRate.Numerator;
        gxFrequency.Frequency = frequency;

        if (resolutionIt != gxOutput.DisplayModes.end())
        {
            bool frequencyExists = std::any_of(
                resolutionIt->Frequencies.begin(),
                resolutionIt->Frequencies.end(),
                [&gxFrequency](const GXFREQUENCY& freq) {
                    return freq.Numerator == gxFrequency.Numerator &&
                        freq.Denominator == gxFrequency.Denominator;
                }
            );

            if (!frequencyExists)
                resolutionIt->Frequencies.push_back(gxFrequency);
        }
        else
        {
            gxMode.Frequencies.push_back(gxFrequency);
            gxOutput.DisplayModes.push_back(gxMode);
        }
    }

    return hr;
}




