#pragma once
#include <vector>
#include <unordered_map>
#include <DirectXMath.h>

class Shader;
class Material;
class Mesh;
class Surface;

struct DrawEntry {
    Mesh*                  mesh     = nullptr;
    Surface*               surface  = nullptr;
    DirectX::XMMATRIX      world    = DirectX::XMMatrixIdentity();
};

struct MaterialBatch {
    Material*              material = nullptr;
    std::vector<DrawEntry> draws;
};

struct ShaderBatch {
    Shader*                shader      = nullptr;
    int                    flagsVertex = 0;
    std::vector<MaterialBatch>           materials;
    std::unordered_map<Material*, size_t> matIndex;
};

struct RenderQueue {
    std::vector<ShaderBatch>             shaders;
    std::unordered_map<Shader*, size_t>  shIndex;

    void Clear()
    {
        shaders.clear();
        shIndex.clear();
    }

    // Einen DrawEntry in den richtigen Shader/Material-Bucket einsortieren
    void Submit(Shader* shader, int flagsVertex, Material* material,
                Mesh* mesh, Surface* surface, const DirectX::XMMATRIX& world)
    {
        // Shader-Bucket suchen oder anlegen
        auto shIt = shIndex.find(shader);
        size_t si;
        if (shIt == shIndex.end())
        {
            si = shaders.size();
            shIndex[shader] = si;
            shaders.push_back({shader, flagsVertex, {}, {}});
        }
        else si = shIt->second;

        ShaderBatch& sb = shaders[si];

        // Material-Bucket suchen oder anlegen
        auto matIt = sb.matIndex.find(material);
        size_t mi;
        if (matIt == sb.matIndex.end())
        {
            mi = sb.materials.size();
            sb.matIndex[material] = mi;
            sb.materials.push_back({material, {}});
        }
        else mi = matIt->second;

        sb.materials[mi].draws.push_back({mesh, surface, world});
    }
};

