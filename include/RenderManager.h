#pragma once
#include "ObjectManager.h"
#include "TexturePool.h"
#include "Dx11LightManagerGpuData.h"
#include "LightArrayBuffer.h"
#include "RenderQueue.h"
#include "gdxdevice.h"
#include "ShadowMapTarget.h"
#include "BackbufferTarget.h"
#include "RenderTextureTarget.h"
#include <d3d11.h>
#include <memory>

class IRenderBackend;
class Dx11RenderBackend;

class RenderManager
{
public:
    RenderManager(ObjectManager& objectManager, GDXDevice& device);
    ~RenderManager();

    void SetCamera(LPENTITY camera);
    void SetDirectionalLight(LPENTITY dirLight);
    void RenderScene();

    // Phase 4: Shadow Mapping 2-Pass
    void RenderShadowPass();
    void RenderNormalPass();

    // RTT: Render-to-Texture Unterstützung
    // Setzt den aktiven RTT-Target und optional eine andere Kamera für den Pass.
    // Wenn rtt == nullptr, wird bei RenderNormalPass wieder der Backbuffer verwendet.
    void SetRTTTarget(RenderTextureTarget* rtt, LPENTITY rttCamera = nullptr);

    void EnsureBackend();
    void SetTexturePool(TexturePool* pool) noexcept { m_texturePool = pool; }

private:
    RenderQueue m_opaque;

    // Transparent-Queue: vorbereitet, noch nicht aktiv
    // Transparent-Queue: nach Tiefe sortiert (depth, RenderCommand)
    std::vector<std::pair<float, RenderCommand>> m_transFrame;

    // Objekte im 3D-Raum
    LPENTITY m_currentCam;
    LPENTITY m_directionLight;

    // Manager-Klassen (Referenzen)
    ObjectManager& m_objectManager;
    Dx11LightManagerGpuData m_lightGpuData;
    LightArrayBuffer        m_lightCBData;
    GDXDevice& m_device;

    std::unique_ptr<IRenderBackend> m_backend;

    // Render Targets
    ShadowMapTarget  m_shadowTarget;
    BackbufferTarget m_backbufferTarget;

    // TexturePool (non-owning, gesetzt von GDXEngine nach Init)
    TexturePool* m_texturePool = nullptr;

    // Default-Sampler fuer gSampler (s0) – linear wrap
    ID3D11SamplerState* m_defaultSampler = nullptr;

    // Blend States fuer Transparenz
    ID3D11BlendState* m_alphaBlendState = nullptr;  // SRC_ALPHA / INV_SRC_ALPHA
    ID3D11BlendState* m_noBlendState = nullptr;  // Standard: Blending aus

    // Caching: zuletzt gebundene SRVs fuer feste PS-Slots.
    // t0=Albedo, t1=Normal, t2=ORM, t3=Decal, t4=Occlusion, t5=Roughness, t6=Metallic
    // (Separate PBR-Maps sind optional; Shader nutzt weiterhin ORM, wenn MF_USE_ORM_MAP gesetzt ist.)
    ID3D11ShaderResourceView* m_boundSRVs[7] = {};

    // RTT-Support: wenn gesetzt, rendert RenderNormalPass in dieses Target (statt Backbuffer)
    RenderTextureTarget* m_activeRTT = nullptr;
    LPENTITY             m_rttCamera = nullptr;  // optionale RTT-Kamera (non-owning)

    // Helper Functions
    void RenderMainPassAtomic();
    void BuildRenderQueue();
    void FlushRenderQueue();
    void FlushTransparentQueue();
    void UpdateShadowMatrixBuffer(const DirectX::XMMATRIX& viewMatrix,
        const DirectX::XMMATRIX& projMatrix);
    void InvalidateFrame();
    RenderManager() = delete;
};
