#pragma once

class IRenderTarget
{
public:
    virtual ~IRenderTarget() = default;

    // Schritt 2: Bind/Clear/Viewport werden im Backend gekapselt.
    // RenderTargets sind ab hier nur noch "Daten/Handles".
    //
    // Optionaler API-neutraler Zugriff auf eine Shader-Resource-View.
    // In DX11 ist das intern ein ID3D11ShaderResourceView*.
    // Default: nullptr (z.B. BackbufferTarget hat keinen SRV)
    virtual void* GetNativeShaderResourceView() const { return nullptr; }
};
