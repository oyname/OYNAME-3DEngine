#pragma once
#include <DirectXMath.h>

class Shader;
class Material;
class Mesh;
class Surface;
class GDXDevice;
class IRenderBackend;

// Ein RenderCommand beschreibt einen einzelnen Draw-Aufruf vollstaendig.
// Er ist selbst ausfuehrbar (Execute) und sortierbar (SortKey).
//
// Vorteile gegenueber DrawEntry in verschachtelten Buckets:
//   - Flache Liste: einfach sortierbar nach Shader, Material, Tiefe
//   - Unabhaengig vom Aufbau der RenderQueue
//   - Transparent-Pass: nach Tiefe sortieren ohne Umbau der Queue-Struktur
//   - Shadow-Pass: eigene Command-Liste mit anderem Shader, selbe Meshes
struct RenderCommand
{
    // Was gezeichnet wird
    Mesh*                  mesh        = nullptr;
    Surface*               surface     = nullptr;
    DirectX::XMMATRIX      world       = DirectX::XMMatrixIdentity();

    // Wie gezeichnet wird
    Shader*                shader      = nullptr;
    Material*              material    = nullptr;
    int                    flagsVertex = 0;

    // Backend-Hook (API-neutral). RenderCommand selbst bleibt frei von DX11/VK Calls.
    IRenderBackend*        backend     = nullptr;

    // Sortierkriterium: Shader-Pointer -> Material-Pointer -> Tiefe
    // Minimiert GPU-State-Wechsel beim geordneten Flush.
    uint64_t SortKey() const noexcept
    {
        // Obere 32 Bit: Shader-Adresse (niedrige Bits), untere 32 Bit: Material-Adresse
        uint64_t sh = (reinterpret_cast<uintptr_t>(shader)   & 0xFFFFFFFF);
        uint64_t mt = (reinterpret_cast<uintptr_t>(material) & 0xFFFFFFFF);
        return (sh << 32) | mt;
    }

    // Fuehrt den Draw-Call aus. Wird von RenderManager::FlushRenderQueue aufgerufen.
    // Voraussetzung: Shader und Material sind bereits gebunden (State-Batch-Logik
    // im Flush erkennt Wechsel anhand des vorherigen Commands).
    void Execute(const GDXDevice* device) const;
};
