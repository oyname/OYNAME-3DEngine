#include "InputLayoutManager.h"

InputLayoutManager::InputLayoutManager() : m_device(nullptr) {}

InputLayoutManager::~InputLayoutManager() {}

void InputLayoutManager::Init(ID3D11Device* device)
{
    m_device = device;
}

HRESULT InputLayoutManager::CreateInputLayoutVertex(ID3D11InputLayout** layout, SHADER* shader, DWORD& saveFlags, DWORD flags)
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> layoutElements;

    // Can store the size of the vertex
    unsigned int currentOffset = 0;
    // Counts the elements
    unsigned int cnt = 0;

    // Save setting for later use
    saveFlags = flags;

    if (flags & D3DVERTEX_POSITION) {
        layoutElements.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, currentOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        currentOffset += sizeof(DirectX::XMFLOAT3); 
        cnt++;
    }

    if (flags & D3DVERTEX_NORMAL) {
        layoutElements.push_back({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, cnt, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        currentOffset += sizeof(DirectX::XMFLOAT3); 
        cnt++;
    }

    if (flags & D3DVERTEX_TANGENT) {
        layoutElements.push_back({ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, cnt, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        currentOffset += sizeof(DirectX::XMFLOAT4);
        cnt++;
    }

    if (flags & D3DVERTEX_COLOR) {
        layoutElements.push_back({ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, cnt, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        currentOffset += sizeof(DirectX::XMFLOAT4); 
        cnt++;
    }
    
    if (flags & D3DVERTEX_TEX1) {
        layoutElements.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, cnt, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        currentOffset += sizeof(DirectX::XMFLOAT2); 
        cnt++;
    }
   
    if (flags & D3DVERTEX_TEX2) {
        layoutElements.push_back({ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, cnt, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        currentOffset += sizeof(DirectX::XMFLOAT2); 
        cnt++;
    }
    if (flags & D3DVERTEX_BONE_INDICES) {
        layoutElements.push_back({ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, cnt, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        cnt++;
    }
    if (flags & D3DVERTEX_BONE_WEIGHTS) {
        layoutElements.push_back({ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, cnt, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 });
        cnt++;
    }

    void* bytecode = shader->blobVS->GetBufferPointer();
    unsigned int size = (unsigned int)shader->blobVS->GetBufferSize();

    if (!shader || !shader->blobVS || shader->blobVS->GetBufferSize() == 0)
    {
        Debug::Log("CreateInputLayout: invalid VS blob");
        return E_INVALIDARG;
    }

    HRESULT hr = m_device->CreateInputLayout(layoutElements.data(), (unsigned int)layoutElements.size(), bytecode, size, layout);
    if (FAILED(hr))
    {
        Debug::LogHr(__FILE__, __LINE__, hr);
        return hr;
    }

    return hr;
}