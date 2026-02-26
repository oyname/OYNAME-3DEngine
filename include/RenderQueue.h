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
// Sort() ordnet nach SortKey (Shader -> Material), um State-Wechsel
// auf der GPU zu minimieren -- dasselbe Ziel wie die Bucket-Struktur,
// aber flexibler (z.B. Tiefensortierung fuer Transparenz moeglich).
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
        cmd.mesh        = mesh;
        cmd.surface     = surface;
        cmd.world       = world;
        cmd.shader      = shader;
        cmd.material    = material;
        cmd.flagsVertex = flagsVertex;
        cmd.backend     = backend;
        commands.push_back(cmd);
    }

    // Sortiert nach Shader, dann Material -- minimiert GPU-State-Wechsel.
    void Sort()
    {
        std::sort(commands.begin(), commands.end(),
            [](const RenderCommand& a, const RenderCommand& b) {
                return a.SortKey() < b.SortKey();
            });
    }

    size_t Count() const noexcept { return commands.size(); }
};
