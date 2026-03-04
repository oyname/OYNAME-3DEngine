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
    Mesh* mesh = nullptr;
    Surface* surface = nullptr;
    DirectX::XMMATRIX      world = DirectX::XMMatrixIdentity();

    // Wie gezeichnet wird
    Shader* shader = nullptr;
    Material* material = nullptr;
    int                    flagsVertex = 0;

    // Backend-Hook (API-neutral). RenderCommand selbst bleibt frei von DX11/VK Calls.
    IRenderBackend* backend = nullptr;

    // Sortierkriterium (legacy): 64-bit Key aus Shader/Material.
    // WICHTIG: Auf 64-bit Systemen passen zwei Pointer nicht verlustfrei in einen uint64_t.
    // Deshalb liefert SortKey nur einen *gemischten* Key ohne 32-bit Truncation.
    // Fuer eine kollisionsfreie Ordnung muss der Comparator (z.B. RenderQueue::Sort)
    // direkt die Pointer (uintptr_t) lexikographisch vergleichen.
    uint64_t SortKey() const noexcept
    {
        // 64-bit Mix 
        auto mix64 = [](uint64_t x) noexcept {
            x ^= x >> 33;
            x *= 0xff51afd7ed558ccdULL;
            x ^= x >> 33;
            x *= 0xc4ceb9fe1a85ec53ULL;
            x ^= x >> 33;
            return x;
            };

        uint64_t sh = mix64(static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(shader)));
        uint64_t mt = mix64(static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(material)));

        // Kombinieren (boost::hash_combine Stil)
        return sh ^ (mt + 0x9e3779b97f4a7c15ULL + (sh << 6) + (sh >> 2));
    }

    // Fuehrt den Draw-Call aus. Wird von RenderManager::FlushRenderQueue aufgerufen.
    // Voraussetzung: Shader und Material sind bereits gebunden (State-Batch-Logik
    // im Flush erkennt Wechsel anhand des vorherigen Commands).
    void Execute(const GDXDevice* device) const;
};
