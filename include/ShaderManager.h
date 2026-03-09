#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include <list>
#include <unordered_map>
#include "gdxutil.h"
#include "Shader.h"

#define SHADER_FOLDER L"shaders\\"

enum class ShaderKey
{
    Standard = 0,
    StandardSkinned = 1,
    Shadow = 2,          // VS-only shadow pass shader (reads b0 world + b3 light view/proj)
};

struct ShaderKeyHash
{
    std::size_t operator()(const ShaderKey key) const noexcept
    {
        return static_cast<std::size_t>(key);
    }
};

class ShaderManager {
public:
    ShaderManager();

    void Init(ID3D11Device* device);
    HRESULT CreateShader(SHADER* shader, const std::wstring& vertexShaderFile, const std::string& vertexEntryPoint, const std::wstring& pixelShaderFile, const std::string& pixelEntryPoint);
    HRESULT CompileShaderFromFile(const std::wstring& filename, const std::string& entryPoint, const std::string& shaderModel, ID3DBlob** blob);
    LPSHADER GetShader();
    LPSHADER GetShader(ShaderKey key) const;
    void SetShader(LPSHADER shader);
    void SetShader(ShaderKey key, LPSHADER shader);
    bool HasShader(ShaderKey key) const;

private:
    ID3D11Device* m_device;
    std::unordered_map<ShaderKey, LPSHADER, ShaderKeyHash> m_shaders;
};
