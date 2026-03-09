#include "ShaderManager.h"
#include <fstream>

ShaderManager::ShaderManager() : m_device(nullptr)
{
}

void ShaderManager::Init(ID3D11Device* device)
{
    m_device = device;
}

HRESULT ShaderManager::CreateShader(SHADER* shader, const std::wstring& vertexShaderFile, const std::string& vertexEntryPoint, const std::wstring& pixelShaderFile, const std::string& pixelEntryPoint)
{
    HRESULT hr;
    ID3DBlob* blobVS = nullptr;
    ID3DBlob* blobPS = nullptr;

    // Load and compile the vertex shader
    hr = CompileShaderFromFile(vertexShaderFile, vertexEntryPoint, "vs_5_0", &blobVS);
    if (FAILED(hr)) {
        return hr;
    }

    // Create the vertex shader
    hr = m_device->CreateVertexShader(blobVS->GetBufferPointer(), blobVS->GetBufferSize(), nullptr, &shader->vertexShader);
    if (FAILED(hr))
    {
        DBLOG_HR(hr);
        Memory::SafeRelease(blobVS);
        return hr;
    }

    // Load and compile the pixel shader
    hr = CompileShaderFromFile(pixelShaderFile, pixelEntryPoint, "ps_5_0", &blobPS);
    if (FAILED(hr))
    {
        DBLOG_HR(hr);
        Memory::SafeRelease(blobVS);
        Memory::SafeRelease(shader->vertexShader);
        return hr;
    }

    // Create the pixel shader
    hr = m_device->CreatePixelShader(blobPS->GetBufferPointer(), blobPS->GetBufferSize(), nullptr, &shader->pixelShader);
    if (FAILED(hr))
    {
        DBLOG_HR(hr);
        Memory::SafeRelease(blobVS);
        Memory::SafeRelease(blobPS);
        Memory::SafeRelease(shader->vertexShader);
        return hr;
    }

    // Store blobs for input layout (must be released manually later)
    shader->blobVS = blobVS;
    shader->blobPS = blobPS;

    // Speichere Dateinamen
    shader->vertexShaderFile = vertexShaderFile;
    shader->pixelShaderFile = pixelShaderFile;

    return S_OK;
}

HRESULT ShaderManager::CompileShaderFromFile(const std::wstring& filename, const std::string& entryPoint, const std::string& shaderModel, ID3DBlob** blob)
{
    HRESULT hr;
    ID3DBlob* errorBlob = nullptr;  // Für Compiler-Fehler

    // Compile the shader from file
    hr = D3DCompileFromFile(
        filename.c_str(),
        nullptr,                    // No defines
        nullptr,                    // Standard Include Handler
        entryPoint.c_str(),
        shaderModel.c_str(),
        0,                          // Compile-Flags
        0,                          // Effect-Flags
        blob,
        &errorBlob                  // Error-Blob!
    );

    if (FAILED(hr)) {
        if (errorBlob != nullptr) {
            DBLOG("ShaderManager.cpp: Shader Compilation Error: ", (const char*)errorBlob->GetBufferPointer());
            Memory::SafeRelease(errorBlob);
        }
        return hr;  // return hr, not 0 (0 == S_OK)
    }

    return hr;
}

SHADER* ShaderManager::GetShader()
{
    return GetShader(ShaderKey::Standard);
}

SHADER* ShaderManager::GetShader(ShaderKey key) const
{
    const auto it = m_shaders.find(key);
    return (it != m_shaders.end()) ? it->second : nullptr;
}

void ShaderManager::SetShader(LPSHADER shader)
{
    SetShader(ShaderKey::Standard, shader);
}

void ShaderManager::SetShader(ShaderKey key, LPSHADER shader)
{
    m_shaders[key] = shader;
}

bool ShaderManager::HasShader(ShaderKey key) const
{
    return GetShader(key) != nullptr;
}
