#pragma once
#include "ObjectManager.h"
#include "LightManager.h"
#include "RenderQueue.h"
#include "gdxdevice.h"
#include "ShadowMapTarget.h"
#include "BackbufferTarget.h"
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

private:
    RenderQueue m_opaque;

    // Transparent-Queue: vorbereitet, noch nicht aktiv
    std::vector<std::pair<float, DrawEntry>> m_transFrame;

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

    // Helper Functions
    void BuildRenderQueue();
    void FlushRenderQueue();
    void UpdateShadowMatrixBuffer(const DirectX::XMMATRIX& viewMatrix,
                                  const DirectX::XMMATRIX& projMatrix);

    RenderManager() = delete;
};
