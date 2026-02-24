#pragma once
#include "ObjectManager.h"
#include "LightManager.h"
#include "RenderQueue.h"
#include "gdxdevice.h"
#include "ShadowMapTarget.h"
#include "BackbufferTarget.h"
#include "RenderTextureTarget.h"
#include <d3d11.h>

class RenderManager
{
public:
    RenderManager(ObjectManager& objectManager, LightManager& lightManager, GDXDevice& device);
    ~RenderManager() = default;

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
    LightManager&  m_lightManager;
    GDXDevice&     m_device;

    // Render Targets
    ShadowMapTarget  m_shadowTarget;
    BackbufferTarget m_backbufferTarget;

    // RTT-Support: wenn gesetzt, rendert RenderNormalPass in dieses Target (statt Backbuffer)
    RenderTextureTarget* m_activeRTT    = nullptr;
    LPENTITY             m_rttCamera    = nullptr;  // optionale RTT-Kamera (non-owning)

    // Helper Functions
    void BuildRenderQueue();
    void FlushRenderQueue();
    void UpdateShadowMatrixBuffer(const DirectX::XMMATRIX& viewMatrix,
                                  const DirectX::XMMATRIX& projMatrix);

    RenderManager() = delete;
};
