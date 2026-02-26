#pragma once

// DX11-only helper that owns all shadow map resources.
// Moved out of GDXDevice to keep the public engine flow/API stable.

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11Buffer;

#include <windows.h> // UINT

class Dx11ShadowMap
{
public:
    Dx11ShadowMap() = default;
    ~Dx11ShadowMap();

    // Creates all shadow resources for a square shadow map of given size.
    bool Create(ID3D11Device* dev, UINT size);
    void Release();

    // Bind DSV, clear depth, set viewport and raster state.
    void Begin(ID3D11DeviceContext* ctx);
    void End(ID3D11DeviceContext* ctx); // optional, can be empty

    ID3D11ShaderResourceView* GetSRV()        const { return m_shadowSRV; }
    ID3D11SamplerState*       GetSampler()   const { return m_shadowSampler; }
    ID3D11RasterizerState*    GetRasterState() const { return m_shadowRS; }
    ID3D11Buffer*             GetMatrixCB()  const { return m_shadowMatrixCB; }
    UINT                      GetSize()      const { return m_shadowSize; }
    ID3D11DepthStencilView*   GetDSV()       const { return m_shadowDSV; }

private:
    // Owned COM resources
    ID3D11Texture2D*          m_shadowTex       = nullptr;
    ID3D11DepthStencilView*   m_shadowDSV       = nullptr;
    ID3D11ShaderResourceView* m_shadowSRV       = nullptr;
    ID3D11SamplerState*       m_shadowSampler   = nullptr;
    ID3D11RasterizerState*    m_shadowRS        = nullptr;
    ID3D11Buffer*             m_shadowMatrixCB  = nullptr;

    UINT m_shadowSize = 0;

    // Cached viewport (square)
    float m_vpW = 0.0f;
    float m_vpH = 0.0f;
};
