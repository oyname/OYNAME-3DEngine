#pragma once

class GDXDevice; // forward

// Abstrakte Schnittstelle fuer GPU-seitige Ressourcen einer Surface.
// RenderManager arbeitet ausschliesslich gegen dieses Interface --
// nie gegen konkrete API-Typen wie SurfaceGpuBuffer.
//
// Neue Graphics-API (Vulkan, DX12, Metal):
//   neue Klasse implementiert IGpuResource, Surface bleibt unveraendert.
class IGpuResource
{
public:
    virtual ~IGpuResource() = default;

    // Bindet Vertex-/Indexbuffer und setzt den Draw-Call ab.
    virtual void Draw(const GDXDevice* device, unsigned int flagsVertex) const = 0;

    // Gibt alle GPU-Ressourcen frei (Buffer-Objekte, Handles etc.)
    virtual void Release() = 0;

    // Render-State: Wireframe statt Solid
    virtual void SetWireframe(bool enabled) noexcept = 0;
    virtual bool IsWireframe()              const noexcept = 0;
};
