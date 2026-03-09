#pragma once
#include "Scene.h"
#include "AssetManager.h"
#include "TexturePool.h"
#include "Dx11LightManagerGpuData.h"
#include "LightArrayBuffer.h"
#include "RenderQueue.h"
#include "gdxdevice.h"
#include "ShadowMapTarget.h"
#include "BackbufferTarget.h"
#include "RenderTextureTarget.h"
#include <memory>

// No <d3d11.h> here. All DX11 types live in Dx11RenderBackend.

class IRenderBackend;
class Dx11RenderBackend;

class RenderManager
{
public:
    struct FrameStats
    {
        unsigned int opaqueDrawCalls        = 0;
        unsigned int transparentDrawCalls   = 0;
        unsigned int shadowDrawCalls        = 0;
        unsigned int shaderBinds            = 0;
        unsigned int materialBinds          = 0;
        unsigned int entityUploads          = 0;
        unsigned int entityConstantBinds    = 0;
        unsigned int entityRingRotations    = 0;

        bool operator==(const FrameStats& other) const noexcept
        {
            return opaqueDrawCalls      == other.opaqueDrawCalls      &&
                   transparentDrawCalls == other.transparentDrawCalls &&
                   shadowDrawCalls      == other.shadowDrawCalls      &&
                   shaderBinds          == other.shaderBinds          &&
                   materialBinds        == other.materialBinds        &&
                   entityUploads        == other.entityUploads        &&
                   entityConstantBinds  == other.entityConstantBinds  &&
                   entityRingRotations  == other.entityRingRotations;
        }

        bool operator!=(const FrameStats& other) const noexcept
        {
            return !(*this == other);
        }
    };

    RenderManager(Scene& scene, AssetManager& assetManager, GDXDevice& device);
    ~RenderManager();

    void SetCamera(LPENTITY camera);
    void SetDirectionalLight(LPENTITY dirLight);
    void RenderScene();

    void RenderShadowPass();
    void RenderNormalPass();

    // Sets the active RTT target and optional camera for the pass.
    // Pass nullptr to render to the backbuffer again.
    void SetRTTTarget(RenderTextureTarget* rtt, LPENTITY rttCamera = nullptr);

    void EnsureBackend();
    void SetTexturePool(TexturePool* pool) noexcept { m_texturePool = pool; }
    void SetShadowShader(Shader* shader)   noexcept { m_shadowShader = shader; }
    const FrameStats& GetFrameStats() const noexcept { return m_frameStats; }

private:
    RenderQueue m_opaque;
    RenderQueue m_shadow;

    std::vector<std::pair<float, RenderCommand>> m_transFrame;

    LPENTITY m_currentCam;
    LPENTITY m_directionLight;

    Scene&        m_scene;
    AssetManager& m_assetManager;
    Dx11LightManagerGpuData m_lightGpuData;
    LightArrayBuffer        m_lightCBData;
    GDXDevice&    m_device;

    std::unique_ptr<IRenderBackend> m_backend;

    ShadowMapTarget  m_shadowTarget;
    BackbufferTarget m_backbufferTarget;

    // Non-owning pointer to the shared TexturePool (set by GDXEngine after init).
    TexturePool* m_texturePool = nullptr;

    // Shadow-pass VS (VS-only, b0=world, b3=lightViewProj). Non-owning.
    Shader* m_shadowShader = nullptr;

    // RTT support
    RenderTextureTarget* m_activeRTT  = nullptr;
    LPENTITY             m_rttCamera  = nullptr;

    bool       m_flushOnce = false;
    FrameStats m_frameStats{};
    FrameStats m_lastLoggedFrameStats{};
    bool       m_hasLastLoggedFrameStats = false;

    // Helper functions
    void RenderMainPassAtomic();
    void BuildRenderQueue();
    void BuildShadowQueue();
    void FlushRenderQueue();
    void FlushShadowQueue(const DirectX::XMMATRIX& lightViewMatrix,
                          const DirectX::XMMATRIX& lightProjMatrix);
    void FlushTransparentQueue();
    void UpdateShadowMatrixBuffer(const DirectX::XMMATRIX& viewMatrix,
                                  const DirectX::XMMATRIX& projMatrix);
    void InvalidateFrame();
    void LogFrameStatsIfChanged();

    RenderManager() = delete;
};
