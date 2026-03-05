#pragma once

#include <d3d11.h>
#include <vector>
#include "gdxutil.h"
#include "ObjectManager.h"

class BufferManager
{
private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;

public:
    BufferManager(); 
        
    void Init(ID3D11Device* device, ID3D11DeviceContext* context);
    
    HRESULT CreateBuffer(const void* data, UINT size, UINT count, D3D11_BIND_FLAG bindFlags, ID3D11Buffer** buffer);

    HRESULT UpdateConstantBuffer(ID3D11Buffer* buffer, const void* data, UINT dataSize);
    void UpdateBuffer(ID3D11Buffer* buffer, const void* data, UINT dataSize);
};