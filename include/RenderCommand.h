#pragma once
#include <DirectXMath.h>
#include <cstdint>

class Shader;
class Material;
class Mesh;
class Surface;
class GDXDevice;
class IRenderBackend;

// Ein RenderCommand beschreibt einen einzelnen Draw-Aufruf vollstaendig.
// Er ist selbst ausfuehrbar (Execute) und wird von RenderQueue::Sort()
// nach Shader- und Material-Pointer sortiert.
//
// Vorteile gegenueber DrawEntry in verschachtelten Buckets:
//   - Flache Liste: einfach sortierbar nach Shader, Material, Tiefe
//   - Unabhaengig vom Aufbau der RenderQueue
//   - Transparent-Pass: nach Tiefe sortieren ohne Umbau der Queue-Struktur
//   - Shadow-Pass: eigene Command-Liste mit anderem Shader, selbe Meshes
struct RenderCommand
{
    // Was gezeichnet wird
    Mesh* mesh = nullptr;
    Surface* surface = nullptr;
    DirectX::XMMATRIX      world = DirectX::XMMatrixIdentity();

    // Wie gezeichnet wird
    Shader* shader = nullptr;
    Material* material = nullptr;
    int                    flagsVertex = 0;

    // Backend-Hook (API-neutral). RenderCommand selbst bleibt frei von DX11/VK Calls.
    IRenderBackend* backend = nullptr;

    // Fuehrt den Draw-Call aus. Wird von RenderManager::FlushRenderQueue aufgerufen.
    // Voraussetzung: Shader und Material sind bereits gebunden (State-Batch-Logik
    // im Flush erkennt Wechsel anhand des vorherigen Commands).
    void Execute(const GDXDevice* device) const;
};
