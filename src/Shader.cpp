// Shader.cpp
#include "Shader.h"
#include "gdxdevice.h"

Shader::Shader() :
    isActive(false),
    flagsVertex(0),
    inputlayoutVertex(nullptr),
    vertexShader(nullptr),
    pixelShader(nullptr),
    blobVS(nullptr),
    blobPS(nullptr)
{
    // materials ist vector - keine Initialisierung nötig
}

Shader::~Shader() {
    // Gib alle COM-Objekte frei
    Memory::SafeRelease(inputlayoutVertex);
    Memory::SafeRelease(vertexShader);
    Memory::SafeRelease(pixelShader);
    Memory::SafeRelease(blobVS);
    Memory::SafeRelease(blobPS);

    // materials Vector wird automatisch aufgeräumt
    // Die Material-Objekte selbst werden vom ObjectManager verwaltet, nicht hier löschen!
    materials.clear();
}

void Shader::UpdateShader(const GDXDevice* device, ShaderBindMode mode)
{
    // Fehlerbehandlung: Prüfe auf nullptr
    if (device == nullptr) {
        Debug::Log("Shader.cpp: ERROR: Shader::UpdateShader - device is nullptr");
        return;
    }

    ID3D11DeviceContext* context = device->GetDeviceContext();
    if (context == nullptr) {
        Debug::Log("Shader.cpp: ERROR: Shader::UpdateShader - device context is nullptr");
        return;
    }

    // Prüfe ob Shader für den gewünschten Bind-Mode gültig ist
    if (!IsValid(mode)) {
        Debug::Log("Shader.cpp: WARNING: Shader::UpdateShader - Shader not valid for requested bind mode");
        Debug::Log("Shader.cpp:   mode: ", (mode == ShaderBindMode::VS_ONLY ? "VS_ONLY" : "VS_PS"));
        Debug::Log("Shader.cpp:   inputlayoutVertex: ", (inputlayoutVertex != nullptr ? "OK" : "MISSING"));
        Debug::Log("Shader.cpp:   vertexShader: ", (vertexShader != nullptr ? "OK" : "MISSING"));
        Debug::Log("Shader.cpp:   pixelShader: ", (pixelShader != nullptr ? "OK" : "MISSING"));
        return;
    }

    // Setze Input Layout
    context->IASetInputLayout(inputlayoutVertex);

    // Setze Vertex Shader (immer)
    context->VSSetShader(vertexShader, nullptr, 0);

    // Pixel Shader abhängig vom Pass
    if (mode == ShaderBindMode::VS_ONLY)
    {
        // deterministisch: Depth/Shadow Pass ohne Pixel Shader
        context->PSSetShader(nullptr, nullptr, 0);
    }
    else
    {
        context->PSSetShader(pixelShader, nullptr, 0);
    }

    // Markiere als aktiv
    isActive = true;
}


