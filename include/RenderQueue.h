#pragma once
#include <vector>
#include <algorithm>
#include <DirectXMath.h>
#include "RenderCommand.h"
#include "Shader.h"
#include "Material.h"

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

    // Sortiert nach stabiler Shader-ID, dann Material-ID.
    // Minimiert GPU-State-Wechsel ohne Pointer-Truncation auf 64-bit Plattformen.
    // ID 0 bedeutet: Objekt wurde nicht ueber ObjectManager::Create* angelegt –
    // solche Commands landen ans Ende (nach allen gueltigen Batches).
    void Sort()
    {
        std::sort(commands.begin(), commands.end(),
            [](const RenderCommand& a, const RenderCommand& b) {
                const uint32_t as = a.shader   ? a.shader->id   : 0xFFFFFFFFu;
                const uint32_t bs = b.shader   ? b.shader->id   : 0xFFFFFFFFu;
                if (as != bs) return as < bs;

                const uint32_t am = a.material ? a.material->id : 0xFFFFFFFFu;
                const uint32_t bm = b.material ? b.material->id : 0xFFFFFFFFu;
                return am < bm;
            });
    }

    size_t Count() const noexcept { return commands.size(); }
};
