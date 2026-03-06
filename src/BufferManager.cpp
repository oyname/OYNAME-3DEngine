#include "BufferManager.h"


BufferManager::BufferManager() : m_device(nullptr), m_context(nullptr) {}

void BufferManager::Init(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_device = device;
    m_context = context;
}

HRESULT BufferManager::CreateBuffer(const void* data, UINT size, UINT count, D3D11_BIND_FLAG bindFlags, ID3D11Buffer** buffer)
{
    if (!m_device)
    {
        DBERROR("BufferManager.cpp: CreateBuffer - m_device is nullptr. BufferManager::Init has not been called.");
        return E_FAIL;
    }
    if (!data || size == 0 || count == 0 || !buffer)
    {
        DBERROR("BufferManager.cpp: CreateBuffer - invalid parameters (data/size/count/buffer).");
        return E_INVALIDARG;
    }

    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.Usage = (bindFlags & D3D11_BIND_CONSTANT_BUFFER) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = size * count;
    bufferDesc.BindFlags = bindFlags;
    bufferDesc.CPUAccessFlags = (bindFlags & D3D11_BIND_CONSTANT_BUFFER) ? D3D11_CPU_ACCESS_WRITE : 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData;
    ZeroMemory(&initData, sizeof(initData));
    initData.pSysMem = data;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    return m_device->CreateBuffer(&bufferDesc, &initData, buffer);
}

void BufferManager::UpdateBuffer(ID3D11Buffer* buffer, const void* data, UINT dataSize)
{
    if (!buffer || !data || dataSize == 0) {
        DBLOG("BufferManager.cpp: ERROR: UpdateBuffer - invalid input!");
        return;
    }

    m_context->UpdateSubresource(buffer, 0, nullptr, data, dataSize, 0);
}

HRESULT BufferManager::UpdateConstantBuffer(ID3D11Buffer* buffer, const void* data, UINT dataSize)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) {
        return hr;
    }
    
    memcpy(mappedResource.pData, data, dataSize);
    m_context->Unmap(buffer, 0);

    return hr;
}


