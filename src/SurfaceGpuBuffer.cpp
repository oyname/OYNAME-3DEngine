#include "SurfaceGpuBuffer.h"
#include "gdxdevice.h"

SurfaceGpuBuffer::~SurfaceGpuBuffer()
{
    Release();
}

void SurfaceGpuBuffer::Release()
{
    Memory::SafeRelease(positionBuffer);
    Memory::SafeRelease(normalBuffer);
    Memory::SafeRelease(colorBuffer);
    Memory::SafeRelease(uv1Buffer);
    Memory::SafeRelease(uv2Buffer);
    Memory::SafeRelease(indexBuffer);
}

void SurfaceGpuBuffer::Draw(const GDXDevice* device, unsigned int flagsVertex) const
{
    ID3D11Buffer* buffers[5] = {};
    UINT          strides[5] = {};
    UINT          offsets[5] = {};
    UINT          cnt = 0;

    if (flagsVertex & D3DVERTEX_POSITION) { buffers[cnt] = positionBuffer; strides[cnt] = stridePosition; cnt++; }
    if (flagsVertex & D3DVERTEX_NORMAL)   { buffers[cnt] = normalBuffer;   strides[cnt] = strideNormal;   cnt++; }
    if (flagsVertex & D3DVERTEX_COLOR)    { buffers[cnt] = colorBuffer;    strides[cnt] = strideColor;    cnt++; }
    if (flagsVertex & D3DVERTEX_TEX1)     { buffers[cnt] = uv1Buffer;      strides[cnt] = strideUV1;      cnt++; }
    if (flagsVertex & D3DVERTEX_TEX2)     { buffers[cnt] = uv2Buffer;      strides[cnt] = strideUV2;      cnt++; }

    if (cnt > 0)
        device->GetDeviceContext()->IASetVertexBuffers(0, cnt, buffers, strides, offsets);

    device->GetDeviceContext()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (!m_wireframe)
    {
        device->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device->GetDeviceContext()->DrawIndexed(indexCount, 0, 0);
    }
    else
    {
        device->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        device->GetDeviceContext()->Draw(indexCount, 0);
    }
}
