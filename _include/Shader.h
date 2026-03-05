// Shader.h
#pragma once
#include <vector>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include "gdxutil.h"
#include "Material.h"

class GDXDevice;

// Shader-Klasse - Verwaltet Vertex und Pixel Shader mit Blobs und Input Layout
// 
// Memory Management:
// - Destruktor gibt alle COM-Objekte frei (SafeRelease)
// - Blobs werden nur für Input Layout Creation gebraucht, danach optional freigeben
// 
// Verwendung:
// 1. ShaderManager::CreateShader() erstellt Shader-Objekt
// 2. InputLayout wird mit Blobs erstellt
// 3. UpdateShader() setzt den Shader als aktiv

enum class ShaderBindMode
{
    VS_PS,   // Standard: Vertex + Pixel
    VS_ONLY  // Depth/Shadow: nur Vertex, PixelShader wird auf nullptr gesetzt
};

class Shader {
public:
    Shader();
    ~Shader();

    bool isActive;

    // Vertex Format Flags (Bitwise kombiniert)
    // z.B. D3DVERTEX_POSITION | D3DVERTEX_COLOR | D3DVERTEX_NORMAL
    // Wird für Input Layout Creation genutzt
    DWORD flagsVertex;

    std::wstring vertexShaderFile;
    std::wstring pixelShaderFile;

    ID3D11InputLayout* inputlayoutVertex;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;

    /// Vertex Shader Bytecode Blob
    /// Wird benötigt für:
    /// - Input Layout Creation
    /// - Debugging
    /// - Shader Recompilation
    ID3D10Blob* blobVS;
    ID3D10Blob* blobPS;

    // ==================== MATERIAL-VERWALTUNG ====================
    std::vector<Material*> materials;


    // Setzt diesen Shader als aktiv (Input Layout, VS, optional PS)
    // - VS_PS: InputLayout + VS + PS
    // - VS_ONLY: InputLayout + VS, PS wird auf nullptr gesetzt (Depth/Shadow Pass)
    // Fehlerbehandlung: Prüft auf nullptr Pointers

    void UpdateShader(const GDXDevice* device, ShaderBindMode mode = ShaderBindMode::VS_PS);

    // ==================== HILFSMETHODEN ====================

    // Gibt Vertex Shader Bytecode zurück (aus Blob)
    inline void* GetVertexBytecode() const {
        return blobVS ? blobVS->GetBufferPointer() : nullptr;
    }

    // Gibt Größe des Vertex Shader Bytecode zurück
    inline SIZE_T GetVertexBytecodeSize() const {
        return blobVS ? blobVS->GetBufferSize() : 0;
    }


    //Gibt an ob dieser Shader vollständig initialisiert ist – abhängig vom Bind-Mode

    inline bool IsValid(ShaderBindMode mode) const
    {
        // Layout + VS müssen immer da sein
        if (vertexShader == nullptr || inputlayoutVertex == nullptr)
            return false;

        // Optional: Wenn du blobVS zwingend brauchst, hier prüfen
        // if (blobVS == nullptr) return false;

        if (mode == ShaderBindMode::VS_PS)
        {
            if (pixelShader == nullptr)
                return false;
            // Optional: blobPS checken, wenn du es wirklich brauchst
            // if (blobPS == nullptr) return false;
        }

        return true;
    }


    // Backwards compatible: "vollständig" = normaler Renderpass (VS+PS)
    inline bool IsValid() const {
        return IsValid(ShaderBindMode::VS_PS);
    }
};

typedef Shader* LPSHADER;
typedef Shader SHADER;