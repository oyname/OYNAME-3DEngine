#include "SurfaceGpuBuffer.h"
#include <cstdint>
#include "gdxdevice.h"

SurfaceGpuBuffer::~SurfaceGpuBuffer()
{
    Release();
}

void SurfaceGpuBuffer::Release()
{
    Memory::SafeRelease(positionBuffer);
    Memory::SafeRelease(normalBuffer);
    Memory::SafeRelease(tangentBuffer);
    Memory::SafeRelease(colorBuffer);
    Memory::SafeRelease(uv1Buffer);
    Memory::SafeRelease(uv2Buffer);
    Memory::SafeRelease(boneIndexBuffer);
    Memory::SafeRelease(boneWeightBuffer);
    Memory::SafeRelease(indexBuffer);
}

void SurfaceGpuBuffer::Draw(const GDXDevice* device, unsigned int flagsVertex) const
{
    if (!device) return;

    ID3D11DeviceContext* ctx = device->GetDeviceContext();
    if (!ctx) return;

    if (!indexBuffer || indexCount == 0)
    {
        DBLOG_ONCE("SurfaceGpuBuffer::Draw:no-index",
            "SurfaceGpuBuffer::Draw skipped: missing index buffer or indexCount == 0");
        return;
    }

    ID3D11Buffer* buffers[8] = {};
    UINT          strides[8] = {};
    UINT          offsets[8] = {};
    UINT          slot = 0;

    auto bindRequired = [&](bool enabled, ID3D11Buffer* buffer, UINT stride, const char* name)
    {
        if (!enabled) return true;
        if (!buffer || stride == 0)
        {
            DBLOG_ONCE(name, "SurfaceGpuBuffer::Draw skipped: missing required vertex stream ", name);
            return false;
        }

        buffers[slot] = buffer;
        strides[slot] = stride;
        offsets[slot] = 0;
        ++slot;
        return true;
    };

    if (!bindRequired((flagsVertex & D3DVERTEX_POSITION) != 0, positionBuffer,   stridePosition,              "POSITION"))     return;
    if (!bindRequired((flagsVertex & D3DVERTEX_NORMAL) != 0,   normalBuffer,     strideNormal,                "NORMAL"))       return;
    if (!bindRequired((flagsVertex & D3DVERTEX_TANGENT) != 0,  tangentBuffer,    strideTangent,               "TANGENT"))      return;
    if (!bindRequired((flagsVertex & D3DVERTEX_COLOR) != 0,    colorBuffer,      strideColor,                 "COLOR"))        return;
    if (!bindRequired((flagsVertex & D3DVERTEX_TEX1) != 0,     uv1Buffer,        strideUV1,                   "TEXCOORD0"))    return;
    if (!bindRequired((flagsVertex & D3DVERTEX_TEX2) != 0,     uv2Buffer,        strideUV2,                   "TEXCOORD1"))    return;
    if (!bindRequired((flagsVertex & D3DVERTEX_BONE_INDICES) != 0, boneIndexBuffer,  sizeof(uint32_t) * 4,   "BLENDINDICES")) return;
    if (!bindRequired((flagsVertex & D3DVERTEX_BONE_WEIGHTS) != 0, boneWeightBuffer, sizeof(float) * 4,      "BLENDWEIGHT"))  return;

    // Vorherige Bindings sauber leeren, damit keine alten Streams in hoehere Slots hineinragen.
    ID3D11Buffer* nullBuffers[8] = {};
    UINT zeroStrides[8] = {};
    UINT zeroOffsets[8] = {};
    ctx->IASetVertexBuffers(0, 8, nullBuffers, zeroStrides, zeroOffsets);

    if (slot > 0)
        ctx->IASetVertexBuffers(0, slot, buffers, strides, offsets);

    ctx->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (!m_wireframe)
    {
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->DrawIndexed(indexCount, 0, 0);
    }
    else
    {
        // Achtung: Das ist nur eine einfache Linien-Ausgabe auf Basis des vorhandenen Index-Buffers.
        // Echter GPU-Wireframe gehoert in einen Rasterizer-State, nicht in den Topology-Wechsel.
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        ctx->DrawIndexed(indexCount, 0, 0);
    }
}
