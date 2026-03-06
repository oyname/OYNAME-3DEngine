#pragma once
#include <vector>
#include <algorithm>
#include <DirectXMath.h>
#include "RenderCommand.h"

class Shader;
class Material;
class Mesh;
class Surface;
class IRenderBackend;

// RenderQueue: flache Liste von RenderCommands.
//
// Vorher: verschachtelte ShaderBatch -> MaterialBatch -> DrawEntry Hierarchie.
// Jetzt:  jeder Command ist eigenstaendig und traegt alle noetigen Daten.
//
// Sort() ordnet nach Shader- dann Material-Pointer (lexikographisch), um
// State-Wechsel auf der GPU zu minimieren. Dasselbe Ziel wie die alte
// Bucket-Struktur, aber flexibler (z.B. Tiefensortierung fuer Transparenz).
struct RenderQueue
{
    std::vector<RenderCommand> commands;

    void Clear()
    {
        commands.clear();
    }

    void Submit(Shader* shader, int flagsVertex, Material* material,
        Mesh* mesh, Surface* surface, const DirectX::XMMATRIX& world,
        IRenderBackend* backend)
    {
        RenderCommand cmd;
        cmd.mesh = mesh;
        cmd.surface = surface;
        cmd.world = world;
        cmd.shader = shader;
        cmd.material = material;
        cmd.flagsVertex = flagsVertex;
        cmd.backend = backend;
        commands.push_back(cmd);
    }

    // Sortiert nach Shader, dann Material -- minimiert GPU-State-Wechsel.
    void Sort()
    {
        std::sort(commands.begin(), commands.end(),
            [](const RenderCommand& a, const RenderCommand& b) {
                // Kollisionsfreie, 64-bit sichere Ordnung:
                // erst Shader-Pointer, dann Material-Pointer (lexikographisch).
                const auto as = reinterpret_cast<std::uintptr_t>(a.shader);
                const auto bs = reinterpret_cast<std::uintptr_t>(b.shader);
                if (as != bs) return as < bs;

                const auto am = reinterpret_cast<std::uintptr_t>(a.material);
                const auto bm = reinterpret_cast<std::uintptr_t>(b.material);
                return am < bm;
            });
    }

    size_t Count() const noexcept { return commands.size(); }
};
