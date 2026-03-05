// gdxinterface.h
#pragma once

#include "gdxutil.h"
#include <algorithm>


struct GXFREQUENCY {
    unsigned int Denominator;
    unsigned int Numerator;
    float Frequency;
};

struct GXDISPLAYMODE {
    unsigned int Width;
    unsigned int Height;
    std::vector<GXFREQUENCY> Frequencies;
};

struct GXOUTPUT {
    DXGI_OUTPUT_DESC OutputDesc;
    std::vector<GXDISPLAYMODE> DisplayModes;
};

struct GXADAPTER {
    DXGI_ADAPTER_DESC Desc;
    std::vector<GXOUTPUT> Outputs;
};

struct GXINTERFACE
{
public:
    std::vector<GXADAPTER> Adapters;

private:
    DXGI_FORMAT format;                 // DXGI-Format
    float m_maxFrequency;               // FIX: float intern
    unsigned int m_maxNumerator;
    unsigned int m_maxDenominator;
    bool hardwareGPU;

    // ---------- Index-Guards ----------
    bool _ValidAdapter(int adapterindex) const
    {
        return adapterindex >= 0 && adapterindex < (int)Adapters.size();
    }

    bool _ValidOutput(int adapterindex, int monitorindex) const
    {
        if (!_ValidAdapter(adapterindex)) return false;
        return monitorindex >= 0 && monitorindex < (int)Adapters[adapterindex].Outputs.size();
    }

    bool _ValidMode(int adapterindex, int monitorindex, int modus) const
    {
        if (!_ValidOutput(adapterindex, monitorindex)) return false;
        const auto& out = Adapters[adapterindex].Outputs[monitorindex];
        return modus >= 0 && (unsigned int)modus < (unsigned int)out.DisplayModes.size();
    }

    void GetNumeratorDenominator(unsigned int adapterIndex, unsigned int monitorIndex, unsigned int width, unsigned int height)
    {
        m_maxFrequency = 0.0f;
        m_maxNumerator = 0;
        m_maxDenominator = 0;

        if (adapterIndex >= Adapters.size())
            return;

        if (monitorIndex >= Adapters[adapterIndex].Outputs.size())
            return;

        const auto& output = this->Adapters[adapterIndex].Outputs[monitorIndex];
        for (const auto& mode : output.DisplayModes)
        {
            if (mode.Width == width && mode.Height == height)
            {
                for (const auto& freq : mode.Frequencies)
                {
                    float currentFrequency =
                        (freq.Denominator != 0)
                        ? (static_cast<float>(freq.Numerator) / static_cast<float>(freq.Denominator))
                        : 0.0f;

                    if (currentFrequency > m_maxFrequency)
                    {
                        m_maxFrequency = currentFrequency;
                        m_maxNumerator = freq.Numerator;
                        m_maxDenominator = freq.Denominator;
                    }
                }
            }
        }
    }

public:
    GXINTERFACE()
        : Adapters(),
        format(DXGI_FORMAT_UNKNOWN),
        m_maxFrequency(0.0f),
        m_maxNumerator(0),
        m_maxDenominator(0),
        hardwareGPU(false)
    {
        CleanupResources();
        SetDXGI_Format(DXGI_FORMAT_R8G8B8A8_UNORM);
    }

    ~GXINTERFACE() { CleanupResources(); }

    void CleanupResources()
    {
        for (auto& adapter : this->Adapters)
        {
            for (auto& output : adapter.Outputs)
            {
                for (auto& displayMode : output.DisplayModes)
                {
                    displayMode.Frequencies.clear();
                }
                output.DisplayModes.clear();
            }
            adapter.Outputs.clear();
        }
        this->Adapters.clear();

        format = DXGI_FORMAT_UNKNOWN;
        hardwareGPU = false;

        m_maxFrequency = 0.0f;
        m_maxNumerator = 0;
        m_maxDenominator = 0;
    }

    void AddAdapter(const GXADAPTER& adapter)
    {
        this->Adapters.push_back(adapter);
    }

    bool GfxModeExists(int width, int height, int frequency, int adapterindex, int monitorindex)
    {
        if (!_ValidOutput(adapterindex, monitorindex)) return false;

        const GXOUTPUT& output = this->Adapters[adapterindex].Outputs[monitorindex];
        for (const auto& displayMode : output.DisplayModes)
        {
            if ((int)displayMode.Width == width && (int)displayMode.Height == height)
            {
                for (const auto& gfxFrequency : displayMode.Frequencies)
                {
                    // float (z.B. 59.94) -> int Vergleich stabil (gerundet)
                    unsigned int f = (unsigned int)(gfxFrequency.Frequency + 0.5f);
                    if (f == (unsigned int)frequency)
                        return true;
                }
            }
        }
        return false;
    }

    bool GfxModeExists(int width, int height, int adapterindex, int monitorindex)
    {
        if (!_ValidOutput(adapterindex, monitorindex)) return false;

        const GXOUTPUT& output = this->Adapters[adapterindex].Outputs[monitorindex];
        for (const auto& displayMode : output.DisplayModes)
        {
            if ((int)displayMode.Width == width && (int)displayMode.Height == height)
                return true;
        }
        return false;
    }

    size_t GetCountOutput(int adapterindex)
    {
        if (!_ValidAdapter(adapterindex)) return 0;
        return this->GetAdapter((size_t)adapterindex).Outputs.size();
    }

    size_t GetCountDisplayModes(int adapterindex, int monitorindex)
    {
        if (!_ValidOutput(adapterindex, monitorindex)) return 0;
        return this->Adapters[adapterindex].Outputs[monitorindex].DisplayModes.size();
    }

    unsigned int GetGfxModeWidth(int modus, int adapterindex, int monitorindex)
    {
        if (!_ValidMode(adapterindex, monitorindex, modus)) return 0;
        return this->Adapters[adapterindex].Outputs[monitorindex].DisplayModes[modus].Width;
    }

    unsigned int GetGfxModeHeight(int modus, int adapterindex, int monitorindex)
    {
        if (!_ValidMode(adapterindex, monitorindex, modus)) return 0;
        return this->Adapters[adapterindex].Outputs[monitorindex].DisplayModes[modus].Height;
    }

    float GetGfxModeFrequency(unsigned int modus, int adapterindex, int monitorindex) const
    {
        if (!_ValidOutput(adapterindex, monitorindex)) return 0.0f;

        const auto& out = Adapters[adapterindex].Outputs[monitorindex];
        if (modus >= out.DisplayModes.size()) return 0.0f;

        const auto& dm = out.DisplayModes[modus];
        if (dm.Frequencies.empty()) return 0.0f;

        return dm.Frequencies[0].Frequency;
    }

    std::string GetGfxDriverName(int adapterindex)
    {
        if (!_ValidAdapter(adapterindex)) return std::string();
        return GXUTIL::WideToUtf8(this->Adapters[adapterindex].Desc.Description);
    }

    const GXADAPTER& GetAdapter(size_t index) const
    {
        // FIX: nie Adapters[0] wenn leer
        static GXADAPTER dummy = {};

        if (Adapters.empty())
            return dummy;

        if (index < Adapters.size())
            return Adapters[index];

        return Adapters[0];
    }

    size_t GetNumAdapters() const
    {
        return Adapters.size();
    }

    bool GetHardwareGPU()
    {
        return hardwareGPU;
    }

    DXGI_FORMAT GetDXGI_Format()
    {
        return format;
    }

    void SetHardwareGPU(bool support)
    {
        hardwareGPU = support;
    }

    void SetDXGI_Format(DXGI_FORMAT dxgi_format)
    {
        format = dxgi_format;
    }

    unsigned int GetNumerator(unsigned int adapterIndex, unsigned int monitorIndex, unsigned int width, unsigned int height)
    {
        GetNumeratorDenominator(adapterIndex, monitorIndex, width, height);
        return m_maxNumerator;
    }

    unsigned int GetDenominator(unsigned int adapterIndex, unsigned int monitorIndex, unsigned int width, unsigned int height)
    {
        GetNumeratorDenominator(adapterIndex, monitorIndex, width, height);
        return m_maxDenominator;
    }

    // Signatur bleibt gleich, R�ckgabe bleibt unsigned int (gerundet)
    unsigned int GetMaxFrequnecy(unsigned int adapterIndex, unsigned int monitorIndex, unsigned int width, unsigned int height)
    {
        GetNumeratorDenominator(adapterIndex, monitorIndex, width, height);
        return (unsigned int)(m_maxFrequency + 0.5f);
    }
};

class GDXInterface {
private:
    // Main system enumeration
    HRESULT GetSystemInfo();

    // Helper: Enumerate all outputs for an adapter
    HRESULT EnumerateAdapterOutputs(IDXGIAdapter* adapter, GXADAPTER& gxAdapter);

    // Helper: Enumerate all display modes for an output
    HRESULT EnumerateDisplayModes(IDXGIOutput* output, GXOUTPUT& gxOutput);

    // Member variables
    bool m_bInitialized;
    IDXGIFactory* m_factory;

public:
    GXINTERFACE interfaceManager;

public:
    GDXInterface();
    ~GDXInterface();

    HRESULT Init(unsigned int bpp);
    void Release();

    IDXGIFactory* GetFactory() { return m_factory; }
};
