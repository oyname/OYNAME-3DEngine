#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
#include "gdxutil.h"
#include "ObjectManager.h"

class InputLayoutManager {
public:
    InputLayoutManager();
    ~InputLayoutManager();

    void Init(ID3D11Device* device);
    HRESULT CreateInputLayoutVertex(ID3D11InputLayout** layout, SHADER* shader, DWORD& saveFlags, DWORD flags);

private:
    ID3D11Device* m_device;
};