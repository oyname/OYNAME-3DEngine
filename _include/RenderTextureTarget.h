#pragma once
#include "IRenderTarget.h"
#include "Texture.h"
#include <d3d11.h>

class GDXDevice;

// Render-to-Texture Target.
// Verwaltet alle D3D11-Ressourcen selbst (Textur, RTV, SRV, Depth).
// Der interne Texture-Wrapper kann direkt mit Engine::MaterialTexture / EntityTexture
// benutzt werden — er zeigt auf denselben SRV.
class RenderTextureTarget : public IRenderTarget
{
public:
    RenderTextureTarget();
    ~RenderTextureTarget();

    // GDXDevice setzen – wird für Rasterizer-Reset in Bind() benötigt.
    // Muss vor Create() aufgerufen werden.
    void SetDevice(GDXDevice* device) { m_gdxDevice = device; }

    // Alle D3D11-Ressourcen anlegen.
    // width/height: Auflösung der Render-Textur (muss nicht Screen-Größe sein).
    HRESULT Create(ID3D11Device* device, ID3D11DeviceContext* context, UINT width, UINT height);

    // Alle Ressourcen freigeben.
    void Release();

    // Schritt 2: Bind/Clear/Viewport passieren im Backend.
    // RenderTextureTarget liefert nur noch die nativen Handles/Infos.
    void* GetNativeShaderResourceView() const override;

    // Gibt den Texture*-Wrapper zurück, der denselben SRV enthält.
    // Direkt nutzbar mit Engine::MaterialTexture(mat, GetTextureWrapper()).
    Texture* GetTextureWrapper() { return &m_textureWrapper; }

    UINT GetWidth()  const { return m_width; }
    UINT GetHeight() const { return m_height; }

    ID3D11RenderTargetView*   GetRTV() const { return m_rttRTV; }
    ID3D11DepthStencilView*   GetDSV() const { return m_depthView; }
    const float*              GetClearColor() const { return m_clearColor; }

    // Viewport für den RTT-Pass (automatisch auf m_width x m_height gesetzt nach Create)
    D3D11_VIEWPORT GetViewport() const;

    // Clearfarbe (RGBA, Default: schwarz/transparent)
    void SetClearColor(float r, float g, float b, float a = 1.0f);

private:
    GDXDevice* m_gdxDevice = nullptr;  // non-owning – für Rasterizer-Reset
    ID3D11Device* m_device = nullptr;  // non-owning
    ID3D11DeviceContext* m_context = nullptr;  // non-owning

    ID3D11Texture2D* m_rttTexture = nullptr;
    ID3D11RenderTargetView* m_rttRTV = nullptr;
    ID3D11ShaderResourceView* m_rttSRV = nullptr;

    ID3D11Texture2D* m_depthTexture = nullptr;
    ID3D11DepthStencilView* m_depthView = nullptr;

    UINT  m_width = 0;
    UINT  m_height = 0;
    float m_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Leichtgewichtiger Texture-Wrapper: teilt den SRV mit m_rttSRV (non-owning intern)
    Texture m_textureWrapper;
};

typedef RenderTextureTarget* LPRENDERTARGET;