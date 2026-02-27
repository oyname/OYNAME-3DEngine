#pragma once

#include <windows.h>
#include <DirectXMath.h>
#include <fstream>  

#include "gdxengine.h"
#include "SurfaceGpuBuffer.h"

extern Timer Time;

namespace Engine
{
    struct Color
    {
        unsigned int r; // Rot
        unsigned int g; // Grün
        unsigned int b; // Blau
        unsigned int a; // Alpha (Transparenz)

        Color(unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha)
            : r(red), g(green), b(blue), a(alpha) {
        }

        // Überladen des Ungleichheitsoperators !=
        bool operator!=(const Color& other) const {
            return (r != other.r || g != other.g || b != other.b || a != other.a);
        }

        bool operator==(const Color& other) const {
            return (r == other.r && g == other.g && b == other.b && a == other.a);
        }
    };

    // ==================== GRAPHICS INFO ====================

    inline unsigned int CountGfxDrivers() {
        return (unsigned int)engine->m_interface.interfaceManager.GetNumAdapters();
    }

    inline std::string GfxDriverName() {
        return engine->m_interface.interfaceManager.GetGfxDriverName(engine->GetAdapterIndex());
    }

    inline void SetGfxDriver(unsigned int adapter) {
        engine->SetAdapter(adapter);
    }

    inline unsigned int CountOutputs() {
        return (unsigned int)engine->m_interface.interfaceManager.GetCountOutput(engine->GetAdapterIndex());
    }

    inline void SetOutput(unsigned int output) {
        engine->SetOutput(output);
    }

    inline unsigned int CountGfxModes(unsigned int output) {
        return (unsigned int)engine->m_interface.interfaceManager.GetCountDisplayModes(engine->GetAdapterIndex(), output);
    }

    inline unsigned int GfxModeWidth(unsigned int mode) {
        return engine->m_interface.interfaceManager.GetGfxModeWidth(mode, engine->GetAdapterIndex(), engine->GetOutputIndex());
    }

    inline unsigned int GfxModeHeight(unsigned int mode) {
        return engine->m_interface.interfaceManager.GetGfxModeHeight(mode, engine->GetAdapterIndex(), engine->GetOutputIndex());
    }

    inline unsigned int GetGfxModeFrequency(unsigned int mode) {
        return engine->m_interface.interfaceManager.GetGfxModeFrequency(mode, engine->GetAdapterIndex(), engine->GetOutputIndex());
    }

    inline unsigned int GfxModeDepth() {
        return engine->GetColorDepth();
    }

    inline bool GfxModeExists(int width, int height, int frequency) {
        return engine->m_interface.interfaceManager.GfxModeExists(width, height, frequency, engine->GetAdapterIndex(), engine->GetOutputIndex());
    }

    inline unsigned int GfxColorDepth() {
        return engine->GetColorDepth();
    }

    inline unsigned int GetWidth() {
        return engine->GetWidth();
    }

    inline unsigned int GetHeight() {
        return engine->GetHeight();
    }

    inline bool GfxFormatSupported(GXFORMAT format) {
        return engine->m_device.deviceManager.IsFormatSupported(engine->GetAdapterIndex(), format);
    }

    inline unsigned int GfxGetDirectXVersion() {
        return engine->m_device.deviceManager.GetFeatureLevel(engine->GetAdapterIndex());
    }

    inline unsigned int GetCurrentAdapter() {
        return engine->GetAdapterIndex();
    }

    inline int GetMaxFrequency(unsigned int width, unsigned int height) {
        return engine->m_interface.interfaceManager.GetMaxFrequnecy(engine->GetAdapterIndex(), engine->GetOutputIndex(), width, height);
    }

    //
    // Diese Funktionen arbeiten mit allen Entity-Typen (Mesh, Camera, Light)
    //

    // ── Schatten werfen (Mesh-Ebene) ──────────────────────────────────────────
    // Steuert ob ein Mesh im Shadow Pass gerendert wird.
    // false = Mesh wirft keinen Schatten, wird im Shadow Pass übersprungen.
    // Entspricht Unity's MeshRenderer.shadowCastingMode / Unreal's bCastShadow.
    inline void EntityCastShadows(LPENTITY entity, bool enabled)
    {
        if (!entity) return;
        entity->SetCastShadows(enabled);
    }

    // ── Schatten empfangen (Material-Ebene) ───────────────────────────────────
    // Steuert ob ein Material die Shadow Map auswertet.
    // false = Oberfläche wird nicht abgedunkelt, auch wenn ein Schatten drauf fällt.
    // Bleibt auf Material-Ebene, weil es im Pixel Shader ausgewertet wird.
    inline void MaterialReceiveShadows(LPMATERIAL material, bool enabled)
    {
        if (!material) return;
        material->SetReceiveShadows(enabled);
    }

    inline void PositionEntity(LPENTITY entity, float x, float y, float z)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: PositionEntity - entity is nullptr");
            return;
        }
        entity->transform.Position(x, y, z);
    }

    inline void PositionEntity(LPENTITY entity, DirectX::XMVECTOR pos)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: PositionEntity - entity is nullptr");
            return;
        }
        entity->transform.SetPosition(pos);
    }

    inline void MoveEntity(LPENTITY entity, float x, float y, float z, Space mode = Space::Local)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: MoveEntity - entity is nullptr");
            return;
        }
        entity->transform.Move(x, y, z, mode);
    }

    inline void RotateEntity(LPENTITY entity, float fRotateX, float fRotateY, float fRotateZ, Space mode = Space::Local)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: RotateEntity - entity is nullptr");
            return;
        }
        entity->transform.Rotate(fRotateX, fRotateY, fRotateZ, mode);
    }

    inline void RotateEntity(LPENTITY entity, DirectX::XMVECTOR quaternion)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: RotateEntity - entity is nullptr");
            return;
        }
        entity->transform.SetRotationQuaternion(quaternion);
    }

    inline void TurnEntity(LPENTITY entity, float fRotateX, float fRotateY, float fRotateZ, Space mode = Space::Local)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: TurnEntity - entity is nullptr");
            return;
        }
        entity->transform.Turn(fRotateX, fRotateY, fRotateZ, mode);
    }

    inline void ScaleEntity(LPENTITY entity, float x, float y, float z)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: ScaleEntity - entity is nullptr");
            return;
        }
        entity->transform.Scale(x, y, z);
    }

    inline void LookAt(LPENTITY entity, float targetX, float targetY, float targetZ)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: LookAt - entity is nullptr");
            return;
        }
        DirectX::XMVECTOR target = DirectX::XMVectorSet(targetX, targetY, targetZ, 1.0f);
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        entity->transform.LookAt(target, up);
    }

    inline void LookAt(LPENTITY entity, DirectX::XMVECTOR target)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: LookAt - entity is nullptr");
            return;
        }
        DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        entity->transform.LookAt(target, up);
    }

    // ==================== CAMERA ====================

    inline void CreateCamera(LPENTITY* camera)
    {
        if (camera == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateCamera - camera pointer is nullptr");
            return;
        }

        Camera* cam = engine->GetOM().CreateCamera();
        if (cam == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateCamera - Failed to create camera");
            return;
        }

        // Initiale View-Matrix: Kamera bei (0,0,0) schaut nach +Z
        cam->GenerateViewMatrix(
            DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),     // Position
            DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f),     // Zielpunkt (nach vorne)
            DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)      // Up
        );

        cam->GenerateProjectionMatrix(
            DirectX::XMConvertToRadians(60.0f),
            (static_cast<float>(engine->GetWidth()) / static_cast<float>(engine->GetHeight())),
            0.1f, 1000.0f
        );

        cam->GenerateViewport(
            0.0f, 0.0f,
            static_cast<float>(engine->GetWidth()),
            static_cast<float>(engine->GetHeight()),
            0.0f, 1.0f
        );

        *camera = cam;
        engine->SetCamera(cam);
    }

    inline void SetCamera(LPENTITY camera)
    {
        if (camera == nullptr) {
            Debug::Log("gidx.h: ERROR: SetCamera - camera is nullptr");
            return;
        }
        engine->SetCamera(camera);  // engine->SetCamera macht den Cast intern
    }

    inline void PositionLightAtCamera(Light* light, class Camera* camera,
        DirectX::XMVECTOR offset = DirectX::XMVectorZero())
    {
        engine->GetLM().PositionLightAtCamera(light, camera, offset);
        engine->GetLM().Update(&engine->m_device);  // GPU-Buffer aktualisieren
    }

    // ==================== LIGHT ====================

    inline void CreateLight(LPENTITY* light, D3DLIGHTTYPE type)
    {
        if (light == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateLight - light pointer is nullptr");
            return;
        }

        // ← GEÄNDERT: Nutze LightManager statt ObjectManager
        Light* l = engine->GetLM().CreateLight(type);
        if (l == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateLight - Failed to create light");
            return;
        }

        // View/Projection für Shadow-Mapping
        l->GenerateViewMatrix(
            DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
            DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 10.0f),
            DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
        );

        l->GenerateProjectionMatrix(
            DirectX::XMConvertToRadians(90.0f),
            (static_cast<float>(engine->GetWidth()) / static_cast<float>(engine->GetHeight())),
            1.0f, 1000.0f
        );

        l->GenerateViewport(
            0.0f, 0.0f,
            static_cast<float>(engine->GetWidth()),
            static_cast<float>(engine->GetHeight()),
            0.0f, 1.0f
        );

        // Light Buffer erstellen
        HRESULT hr = engine->GetBM().CreateBuffer(
            &l->cbLight,
            sizeof(LightBufferData),
            1,
            D3D11_BIND_CONSTANT_BUFFER,
            &l->lightBuffer
        );

        if (FAILED(hr))
        {
            Debug::LogHr(__FILE__, __LINE__, hr);
            return;
        }

        // Constant Buffer für Matrizen
        hr = engine->GetBM().CreateBuffer(
            &l->matrixSet,
            sizeof(MatrixSet),
            1,
            D3D11_BIND_CONSTANT_BUFFER,
            &l->constantBuffer
        );
        if (FAILED(hr))
        {
            Debug::LogHr(__FILE__, __LINE__, hr);
            return;
        }

        *light = l;
    }

    // Globale Ambient-Farbe (gilt für die ganze Szene)
    inline void SetAmbientColor(float r, float g, float b, float a = 1.0f)
    {
        engine->SetGlobalAmbient(DirectX::XMFLOAT4(r, g, b, a));
    }

    inline void LightColor(LPENTITY light, float r, float g, float b, float a = 1.0f)
    {
        if (light == nullptr) {
            Debug::Log("gidx.h: ERROR: LightColor - light is nullptr");
            return;
        }

        Light* l = (light->IsLight()  ? light->AsLight()  : nullptr);
        if (l == nullptr) {
            Debug::Log("gidx.h: ERROR: LightColor - Entity is not a Light!");
            return;
        }

        l->SetDiffuseColor(DirectX::XMFLOAT4(r, g, b, a));
    }

    // Setzt das Directional Light fuer Shadow Mapping.
    // Muss nach CreateLight aufgerufen werden.
    // Nur EIN Directional Light kann Schatten werfen.
    inline void SetDirectionalLight(LPENTITY light)
    {
        if (light == nullptr) {
            Debug::Log("gidx.h: ERROR - SetDirectionalLight - light is nullptr");
            return;
        }

        Light* l = (light->IsLight()  ? light->AsLight()  : nullptr);
        if (l == nullptr) {
            Debug::Log("gidx.h: ERROR - SetDirectionalLight - Entity is not a Light!");
            return;
        }

        engine->SetDirectionalLight(light);
    }

    inline void CreateMesh(LPENTITY* mesh, MATERIAL* material = nullptr)
    {
        if (mesh == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateMesh - mesh pointer is nullptr");
            return;
        }

        Mesh* m = engine->GetOM().CreateMesh();
        if (m == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateMesh - Failed to create mesh");
            return;
        }

        material = material == nullptr ? engine->GetOM().GetStandardMaterial() : material;

        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateMesh - No valid material available");
            engine->GetOM().DeleteMesh(m);
            return;
        }

        engine->GetOM().AddMeshToMaterial(material, m);

        HRESULT hr = engine->GetBM().CreateBuffer(
            &m->matrixSet,
            sizeof(MatrixSet),
            1,
            D3D11_BIND_CONSTANT_BUFFER,
            &m->constantBuffer
        );
        if (FAILED(hr))
        {
            Debug::LogHr(__FILE__, __LINE__, hr);
            engine->GetOM().DeleteMesh(m);
            return;
        }

        *mesh = m;
    }

    // ==================== SHADER ====================

    inline HRESULT CreateShader(LPSHADER* shader,
        const std::wstring& vertexShaderFile,
        const std::string& vertexEntryPoint,
        const std::wstring& pixelShaderFile,
        const std::string& pixelEntryPoint,
        DWORD flags)
    {
        HRESULT hr = S_OK;

        // 1. Validiere Input-Parameter
        if (shader == nullptr) {
            Debug::Log("gidx.h: ERROR: Engine::CreateShader - shader pointer is nullptr");
            return E_INVALIDARG;
        }

        if (vertexShaderFile.empty() || pixelShaderFile.empty()) {
            Debug::Log("gidx.h: ERROR: Engine::CreateShader - shader file paths are empty");
            return E_INVALIDARG;
        }

        if (engine == nullptr) {
            Debug::Log("gidx.h: ERROR: Engine::CreateShader - engine is nullptr");
            return E_FAIL;
        }

        // 2. Erstelle Shader-Objekt
        *shader = engine->GetOM().CreateShader();
        if (*shader == nullptr) {
            Debug::Log("gidx.h: ERROR: Engine::CreateShader - Failed to create shader object");
            return E_OUTOFMEMORY;
        }

        // 3. Compiliere Vertex und Pixel Shader
        hr = engine->GetSM().CreateShader(*shader,
            vertexShaderFile,
            vertexEntryPoint,
            pixelShaderFile,
            pixelEntryPoint);

        if (FAILED(hr))
        {
            Debug::LogHr(__FILE__, __LINE__, hr);

            engine->GetOM().DeleteShader(*shader);
            return hr;
        }

        // 4. Setze Vertex Format Flags
        (*shader)->flagsVertex = flags;

        // 5. Erstelle Input Layout
        hr = engine->GetILM().CreateInputLayoutVertex(
            &(*shader)->inputlayoutVertex,
            *shader,
            (*shader)->flagsVertex,
            flags
        );
        if (FAILED(hr))
        {
            Debug::LogHr(__FILE__, __LINE__, hr);
            return hr;
        }

        // 6. Freigeben der Blobs nach Input Layout Creation
        if ((*shader)->blobVS != nullptr) {
            (*shader)->blobVS->Release();
            (*shader)->blobVS = nullptr;
        }
        if ((*shader)->blobPS != nullptr) {
            (*shader)->blobPS->Release();
            (*shader)->blobPS = nullptr;
        }

        return S_OK;
    }

    inline DWORD CreateVertexFlags(
        bool hasPosition = true,
        bool hasNormal = false,
        bool hasColor = false,
        bool hasTexCoord1 = false,
        bool hasTexCoord2 = false,
        bool hasTangent = false)
    {
        DWORD flags = 0;
        if (hasPosition)   flags |= D3DVERTEX_POSITION;
        if (hasNormal)     flags |= D3DVERTEX_NORMAL;
        if (hasColor)      flags |= D3DVERTEX_COLOR;
        if (hasTexCoord1)  flags |= D3DVERTEX_TEX1;
        if (hasTexCoord2)  flags |= D3DVERTEX_TEX2;
        if (hasTangent)    flags |= D3DVERTEX_TANGENT;
        return flags;
    }

    // ==================== MATERIAL ====================



    inline void CreateMaterial(LPMATERIAL* material, SHADER* shader = nullptr)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateMaterial - material pointer is nullptr");
            return;
        }

        *material = engine->GetOM().CreateMaterial();
        if (*material == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateMaterial - Failed to create material");
            return;
        }

        // If Shader == nullptr, then default shader
        shader = shader == nullptr ? engine->GetSM().GetShader() : shader;

        if (shader == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateMaterial - No valid shader available");
            engine->GetOM().DeleteMaterial(*material);
            *material = nullptr;
            return;
        }

        engine->GetOM().AddMaterialToShader(shader, *material);

        // Buffer-Anlage gehört in die Engine, nicht in den Wrapper
        HRESULT hr = engine->InitMaterialBuffer(*material);
        if (FAILED(hr))
        {
            Debug::LogHr(__FILE__, __LINE__, hr);
            engine->GetOM().DeleteMaterial(*material);
            *material = nullptr;
            return;
        }
    }

    inline void CreateSurface(LPSURFACE* surface, LPENTITY entity)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateSurface - surface pointer is nullptr");
            return;
        }

        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateSurface - entity is nullptr");
            return;
        }

        *surface = engine->GetOM().CreateSurface();
        if (*surface == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateSurface - Failed to create surface");
            return;
        }

        // Prüfe ob Entity ein Mesh ist (nur Meshes haben Surfaces)
        Mesh* mesh = (entity->IsMesh()   ? entity->AsMesh()   : nullptr);
        if (mesh == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateSurface - Entity is not a Mesh!");
            engine->GetOM().DeleteSurface(*surface);
            *surface = nullptr;
            return;
        }

        engine->GetOM().AddSurfaceToMesh(mesh, *surface);
    }

    inline LPSURFACE GetSurface(LPENTITY entity)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: GetSurface - entity is nullptr");
            return nullptr;
        }

        Mesh* mesh = (entity->IsMesh()   ? entity->AsMesh()   : nullptr);
        if (mesh == nullptr) {
            Debug::Log("gidx.h: ERROR: GetSurface - Entity is not a Mesh!");
            return nullptr;
        }

        return engine->GetOM().GetSurface(mesh);
    }

    inline void FillBuffer(LPSURFACE surface)
    {
        if (!surface) { Debug::Log("gidx.h: ERROR: FillBuffer - surface is nullptr"); return; }

        Shader* shader = nullptr;
        if (surface->pMesh && surface->pMesh->pMaterial)
            shader = surface->pMesh->pMaterial->pRenderShader;

        if (!shader) { Debug::Log("gidx.h: ERROR: FillBuffer - cannot resolve shader"); return; }

        // FillBuffer ist DX11-spezifisch: Cast auf konkreten Typ erlaubt
        SurfaceGpuBuffer* gpuDX11 = static_cast<SurfaceGpuBuffer*>(surface->gpu.get());
        if (!gpuDX11) { Debug::Log("gidx.h: ERROR: FillBuffer - gpu ist kein SurfaceGpuBuffer"); return; }

        // Stride-Werte und Index-Count setzen
        gpuDX11->stridePosition = sizeof(DirectX::XMFLOAT3);
        gpuDX11->strideNormal   = sizeof(DirectX::XMFLOAT3);
        gpuDX11->strideTangent  = sizeof(DirectX::XMFLOAT4);
        gpuDX11->strideColor    = sizeof(DirectX::XMFLOAT4);
        gpuDX11->strideUV1      = sizeof(DirectX::XMFLOAT2);
        gpuDX11->strideUV2      = sizeof(DirectX::XMFLOAT2);
        gpuDX11->indexCount     = surface->CountIndices();

        // Vertexbuffer
        if (shader->flagsVertex & D3DVERTEX_POSITION && surface->CountVertices() > 0) {
            engine->GetBM().CreateBuffer(surface->GetPositions().data(), sizeof(DirectX::XMFLOAT3),
                surface->CountVertices(), D3D11_BIND_VERTEX_BUFFER, &gpuDX11->positionBuffer);
        }
        if (shader->flagsVertex & D3DVERTEX_NORMAL && surface->CountNormals() > 0) {
            engine->GetBM().CreateBuffer(surface->GetNormals().data(), sizeof(DirectX::XMFLOAT3),
                surface->CountNormals(), D3D11_BIND_VERTEX_BUFFER, &gpuDX11->normalBuffer);
        }

        if (shader->flagsVertex & D3DVERTEX_TANGENT)
        {
            // Lazy tangent build (CPU) if missing
            if (surface->CountTangents() != surface->CountVertices())
                surface->ComputeTangents();

            if (surface->CountTangents() > 0)
            {
                engine->GetBM().CreateBuffer(surface->GetTangents().data(), sizeof(DirectX::XMFLOAT4),
                    surface->CountTangents(), D3D11_BIND_VERTEX_BUFFER, &gpuDX11->tangentBuffer);
            }
        }

        if (shader->flagsVertex & D3DVERTEX_COLOR && surface->CountColors() > 0) {
            engine->GetBM().CreateBuffer(surface->GetColors().data(), sizeof(DirectX::XMFLOAT4),
                surface->CountColors(), D3D11_BIND_VERTEX_BUFFER, &gpuDX11->colorBuffer);
        }
        if (shader->flagsVertex & D3DVERTEX_TEX1 && surface->CountUV1() > 0) {
            engine->GetBM().CreateBuffer(surface->GetUV1().data(), sizeof(DirectX::XMFLOAT2),
                surface->CountUV1(), D3D11_BIND_VERTEX_BUFFER, &gpuDX11->uv1Buffer);
        }
        if (shader->flagsVertex & D3DVERTEX_TEX2 && surface->CountUV2() > 0) {
            engine->GetBM().CreateBuffer(surface->GetUV2().data(), sizeof(DirectX::XMFLOAT2),
                surface->CountUV2(), D3D11_BIND_VERTEX_BUFFER, &gpuDX11->uv2Buffer);
        }

        // Indexbuffer
        engine->GetBM().CreateBuffer(surface->GetIndices().data(), sizeof(UINT),
            surface->CountIndices(), D3D11_BIND_INDEX_BUFFER, &gpuDX11->indexBuffer);
    }

    inline void UpdateColorBuffer(LPSURFACE surface)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: UpdateColorBuffer - surface is nullptr");
            return;
        }
        SurfaceGpuBuffer* gpuDX11 = static_cast<SurfaceGpuBuffer*>(surface->gpu.get());
        if (!gpuDX11) return;
        engine->GetBM().UpdateBuffer(gpuDX11->colorBuffer, surface->GetColors().data(), sizeof(DirectX::XMFLOAT4) * surface->CountColors());
    }

    inline void UpdateVertexBuffer(LPSURFACE surface)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: UpdateVertexBuffer - surface is nullptr");
            return;
        }
        SurfaceGpuBuffer* gpuDX11 = static_cast<SurfaceGpuBuffer*>(surface->gpu.get());
        if (!gpuDX11) return;
        engine->GetBM().UpdateBuffer(gpuDX11->positionBuffer, surface->GetPositions().data(), sizeof(DirectX::XMFLOAT3) * surface->CountVertices());
    }

    inline void AddVertex(LPSURFACE surface, float x, float y, float z)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: AddVertex - surface is nullptr");
            return;
        }
        surface->AddVertex(-1, x, y, z);
    }

    inline void AddVertex(int index, LPSURFACE surface, DirectX::XMVECTOR vec)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: AddVertex - surface is nullptr");
            return;
        }
        surface->AddVertex(index, DirectX::XMVectorGetX(vec), DirectX::XMVectorGetY(vec), DirectX::XMVectorGetZ(vec));
    }

    inline void AddVertex(LPSURFACE surface, DirectX::XMVECTOR vec)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: AddVertex - surface is nullptr");
            return;
        }
        surface->AddVertex(-1, DirectX::XMVectorGetX(vec), DirectX::XMVectorGetY(vec), DirectX::XMVectorGetZ(vec));
    }

    inline void AddVertex(LPSURFACE surface, DirectX::XMFLOAT3 vec)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: AddVertex - surface is nullptr");
            return;
        }
        surface->AddVertex(-1, vec.x, vec.y, vec.z);
    }

    inline void AddVertex(int index, LPSURFACE surface, DirectX::XMFLOAT3 vec)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: AddVertex - surface is nullptr");
            return;
        }
        surface->AddVertex(index, vec.x, vec.y, vec.z);
    }

    inline void VertexNormal(LPSURFACE surface, float x, float y, float z)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: VertexNormal - surface is nullptr");
            return;
        }
        surface->VertexNormal(-1, x, y, z);
    }

    inline void VertexNormal(LPSURFACE surface, unsigned int index, float x, float y, float z)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: VertexNormal - surface is nullptr");
            return;
        }
        surface->VertexNormal(index, x, y, z);
    }

    inline void VertexColor(LPSURFACE surface, unsigned int r, unsigned int g, unsigned int b)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: VertexColor - surface is nullptr");
            return;
        }
        surface->VertexColor(-1, float(r / 255.0f), float(g / 255.0f), float(b / 255.0f));
    }

    inline void VertexColor(LPSURFACE surface, unsigned int index, unsigned int r, unsigned int g, unsigned int b)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: VertexColor - surface is nullptr");
            return;
        }
        surface->VertexColor(index, float(r / 255.0f), float(g / 255.0f), float(b / 255.0f));
    }

    inline void VertexTexCoord(LPSURFACE surface, float u, float v)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: VertexTexCoord - surface is nullptr");
            return;
        }
        surface->VertexTexCoords(-1, u, v);
    }

    // Zweite UV-Koordinate (Lightmap / Detail-Map) – benötigt D3DVERTEX_TEX2
    inline void VertexTexCoord2(LPSURFACE surface, float u, float v)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: VertexTexCoord2 - surface is nullptr");
            return;
        }
        surface->VertexTexCoords2(u, v);
    }

    inline void AddTriangle(LPSURFACE surface, unsigned int a, unsigned int b, unsigned int c)
    {
        if (surface == nullptr) {
            Debug::Log("gidx.h: ERROR: AddTriangle - surface is nullptr");
            return;
        }
        surface->AddIndex(a);
        surface->AddIndex(b);
        surface->AddIndex(c);
    }

    // ==================== RENDERING ====================

    inline int Cls(int r, int g, int b, int a = 255)
    {
        return static_cast<int>(engine->Cls(float(r / 255.0f), float(g / 255.0f), float(b / 255.0f), float(a / 255.0f)));
    }

    inline int Flip()
    {
        // nutzt direkt das Interval 0/1
        return static_cast<int>(engine->m_device.Flip(engine->GetVSyncInterval()));
    }

    inline void SetVSync(int interval)
    {
        if (engine) engine->SetVSyncInterval(interval);
    }

    inline int GetVSync()
    {
        return engine ? engine->GetVSyncInterval() : 1;
    }

    inline unsigned int Graphics(unsigned int width, unsigned int height, bool windowed = true)
    {
        return static_cast<int>(engine->Graphic(width, height, windowed));
    }

    inline HRESULT RenderWorld()
    {
        return engine->RenderWorld();
    }

    inline void UpdateWorld()
    {
        engine->UpdateWorld();
    }

    // ==================== TEXTURE ====================

    inline void LoadTexture(LPLPTEXTURE texture, const wchar_t* filename)
    {

        // Prüfe ob Datei existiert
        std::wifstream file(filename);
        if (!file.good()) {
            Debug::Log("gidx.h: ERROR: File not found!");
            return;
        }

        *texture = new TEXTURE;

        HRESULT hr = engine->GetTM().LoadTexture(
            engine->m_device.GetDevice(),
            engine->m_device.GetDeviceContext(),
            filename,
            texture
        );

        if (FAILED(hr)) {
            Debug::Log("gidx.h: ERROR: Failed to load texture");
        }
    }

    // ── MaterialTexture (Legacy-API, bleibt erhalten) ───────────────────────────
    // Speichert Textur im Material-Slot (slot 0..7) und registriert die SRV
    // automatisch im globalen TexturePool.
    inline void MaterialTexture(LPMATERIAL material, LPTEXTURE texture, int slot = 0)
    {
        if (!material || !texture) return;
        // Legacy-Slot-Speicherung bleibt
        material->SetTexture(slot, texture->m_texture,
            texture->m_textureView,
            texture->m_imageSamplerState);
        // TexturePool-Registrierung: SRV bekommt einen stabilen Index
        if (engine && texture->m_textureView)
        {
            uint32_t idx = engine->GetTP().GetOrAdd(texture->m_textureView);
            // Slot-Semantik: 0=Albedo, 1=Normal, 2=ORM, 3=Decal
            switch (slot)
            {
            case 0: material->SetAlbedoIndex(idx); break;
            case 1: material->SetNormalIndex(idx); break;
            case 2: material->SetOrmIndex(idx);    break;
            case 3: material->SetDecalIndex(idx);  break;
            default: break;
            }
        }
    }

    // ── MaterialSetAlbedo ─────────────────────────────────────────────────────
    // Weist dem Material die Albedo-Textur zu und registriert sie im TexturePool.
    inline void MaterialSetAlbedo(LPMATERIAL material, LPTEXTURE texture)
    {
        if (!material) return;
        if (!texture || !texture->m_textureView)
        {
            material->SetAlbedoIndex(engine ? engine->GetTP().WhiteIndex() : 0u);
            return;
        }
        material->SetTexture(0, texture->m_texture, texture->m_textureView, texture->m_imageSamplerState);
        if (engine)
        {
            uint32_t idx = engine->GetTP().GetOrAdd(texture->m_textureView);
            material->SetAlbedoIndex(idx);
            Debug::Log("gidx.h: MaterialSetAlbedo – Index ", idx);
        }
    }

    // ── MaterialSetNormal ─────────────────────────────────────────────────────
    // Weist dem Material die Normal-Map zu. Setzt MF_USE_NORMAL_MAP automatisch.
    inline void MaterialSetNormal(LPMATERIAL material, LPTEXTURE texture)
    {
        if (!material) return;
        if (!texture || !texture->m_textureView)
        {
            material->SetNormalIndex(engine ? engine->GetTP().FlatNormalIndex() : 1u);
            return;
        }
        material->SetTexture(1, texture->m_texture, texture->m_textureView, texture->m_imageSamplerState);
        material->SetNormalScale(material->GetNormalScale() > 0.0001f ? material->GetNormalScale() : 1.0f);
        if (engine)
        {
            uint32_t idx = engine->GetTP().GetOrAdd(texture->m_textureView);
            material->SetNormalIndex(idx);
            Debug::Log("gidx.h: MaterialSetNormal – Index ", idx);
        }
    }

    // ── MaterialSetORM ────────────────────────────────────────────────────────
    // Weist dem Material die ORM-Textur (Occlusion/Roughness/Metallic) zu.
    // Setzt MF_USE_ORM_MAP automatisch.
    inline void MaterialSetORM(LPMATERIAL material, LPTEXTURE texture)
    {
        if (!material) return;
        if (!texture || !texture->m_textureView)
        {
            material->SetOrmIndex(engine ? engine->GetTP().OrmIndex() : 2u);
            return;
        }
        material->SetTexture(2, texture->m_texture, texture->m_textureView, texture->m_imageSamplerState);
        if (engine)
        {
            uint32_t idx = engine->GetTP().GetOrAdd(texture->m_textureView);
            material->SetOrmIndex(idx);
            // ORM-Flag aktivieren
            material->properties.flags |= Material::MF_USE_ORM_MAP;
            Debug::Log("gidx.h: MaterialSetORM – Index ", idx);
        }
    }

    // ── MaterialSetDecal ──────────────────────────────────────────────────────
    // Weist dem Material eine optionale Decal-Textur zu.
    inline void MaterialSetDecal(LPMATERIAL material, LPTEXTURE texture)
    {
        if (!material) return;
        if (!texture || !texture->m_textureView)
        {
            material->SetDecalIndex(engine ? engine->GetTP().WhiteIndex() : 0u);
            return;
        }
        material->SetTexture(3, texture->m_texture, texture->m_textureView, texture->m_imageSamplerState);
        if (engine)
        {
            uint32_t idx = engine->GetTP().GetOrAdd(texture->m_textureView);
            material->SetDecalIndex(idx);
            Debug::Log("gidx.h: MaterialSetDecal – Index ", idx);
        }
    }


    // Blend-Modus für zweite Textur:
    // 0 = off          (Standard, nur Textur 1)
    // 1 = Multiply     (Lightmap, Schatten)
    // 2 = Multiply×2   (Detail-Map, Helligkeitskorrektur)
    // 3 = Additive     (Glühen, Feuer, Licht)
    // 4 = Lerp(Alpha)  (Decals, Aufkleber – nutzt Alpha-Kanal von Textur 2)
    // 5 = Luminanz     (Overlay mit schwarzem Hintergrund)
    inline void MaterialBlendMode(LPMATERIAL material, int mode = 0)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialBlendMode - material is nullptr");
            return;
        }
        material->SetBlendMode(mode);
    }

    inline void MaterialColor(LPMATERIAL material, float r, float g, float b, float a = 1.0f)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialColor - material is nullptr");
            return;
        }
        material->SetDiffuseColor(r, g, b, a);
    }

    inline void MaterialShininess(LPMATERIAL material, float shininess)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialShininess - material is nullptr");
            return;
        }
        material->SetShininess(shininess);
    }


    inline void MaterialMetallic(LPMATERIAL material, float metallic)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialMetallic - material is nullptr");
            return;
        }
        material->SetMetallic(metallic);
    }

    inline void MaterialRoughness(LPMATERIAL material, float roughness)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialRoughness - material is nullptr");
            return;
        }
        material->SetRoughness(roughness);
    }

    inline void MaterialNormalScale(LPMATERIAL material, float scale)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialNormalScale - material is nullptr");
            return;
        }
        material->SetNormalScale(scale);
    }

    inline void MaterialOcclusionStrength(LPMATERIAL material, float strength)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialOcclusionStrength - material is nullptr");
            return;
        }
        material->SetOcclusionStrength(strength);
    }

    inline void MaterialEmissiveColor(LPMATERIAL material, float r, float g, float b, float intensity = 1.0f)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialEmissiveColor - material is nullptr");
            return;
        }
        material->SetEmissiveColor(r, g, b, intensity);
    }

    inline void MaterialUVTilingOffset(LPMATERIAL material, float tileU, float tileV, float offU = 0.0f, float offV = 0.0f)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialUVTilingOffset - material is nullptr");
            return;
        }
        material->SetUVTilingOffset(tileU, tileV, offU, offV);
    }

    inline void MaterialAlphaCutoff(LPMATERIAL material, float cutoff)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialAlphaCutoff - material is nullptr");
            return;
        }
        material->SetAlphaCutoff(cutoff);
        material->SetAlphaTest(true);
    }

    inline void MaterialAlphaTest(LPMATERIAL material, bool enabled)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialAlphaTest - material is nullptr");
            return;
        }
        material->SetAlphaTest(enabled);
    }

    inline void EntityMaterial(LPENTITY entity, LPMATERIAL material)
    {
        Mesh* mesh = (entity->IsMesh()   ? entity->AsMesh()   : nullptr);
        if (!mesh) { Debug::Log("gidx.h: EntityMaterial - Entity ist kein Mesh"); return; }
        if (!material) { Debug::Log("gidx.h: EntityMaterial - material nullptr"); return; }
        engine->GetOM().AddMeshToMaterial(material, mesh);
    }

    inline void MaterialBlendFactor(LPMATERIAL material, float factor)
    {
        if (material == nullptr) {
            Debug::Log("gidx.h: ERROR: MaterialBlendFactor - material is nullptr");
            return;
        }
        material->SetBlendFactor(factor);
    }

    // Weist einer einzelnen Surface ein eigenes Material zu.
    // Ermöglicht mehrere Materialien pro Mesh (z.B. Karosserie, Glas, Reifen).
    inline void SurfaceMaterial(LPSURFACE surface, LPMATERIAL material)
    {
        if (!surface) { Debug::Log("gidx.h: SurfaceMaterial - surface nullptr");  return; }
        if (!material) { Debug::Log("gidx.h: SurfaceMaterial - material nullptr"); return; }
        engine->GetOM().AddMaterialToSurface(material, surface);
    }

    inline void EntityTexture(LPENTITY entity, LPTEXTURE texture)
    {
        if (!entity || !entity->IsMesh())
            return;

        Mesh* mesh = entity->AsMesh();
        if (!mesh || !mesh->pMaterial)
            return;

        MaterialSetAlbedo(mesh->pMaterial, texture);
    }

    inline HRESULT  CreateTexture(LPLPTEXTURE texture, int width, int height)
    {
        if (texture == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateTexture - texture pointer is nullptr");
            return E_INVALIDARG;
        }

        *texture = new TEXTURE;
        if (*texture == nullptr) {
            Debug::Log("gidx.h: ERROR: CreateTexture - Failed to allocate texture");
            return E_OUTOFMEMORY;
        }

        return (*texture)->CreateTexture(engine->m_device.GetDevice(), width, height);
    }

    inline HRESULT LockBuffer(LPTEXTURE texture)
    {
        if (texture == nullptr) {
            Debug::Log("gidx.h: ERROR: LockBuffer - texture is nullptr");
            return E_INVALIDARG;
        }
        return texture->LockBuffer(engine->m_device.GetDeviceContext());
    }

    inline void UnlockBuffer(LPTEXTURE texture)
    {
        if (texture == nullptr) {
            Debug::Log("gidx.h: ERROR: UnlockBuffer - texture is nullptr");
            return;
        }
        texture->UnlockBuffer(engine->m_device.GetDeviceContext());
    }

    inline void SetPixel(LPTEXTURE texture, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char alpha)
    {
        if (texture == nullptr) {
            Debug::Log("gidx.h: ERROR: SetPixel - texture is nullptr");
            return;
        }
        texture->SetPixel(engine->m_device.GetDeviceContext(), x, y, r, g, b, alpha);
    }

    inline Color GetColor(LPTEXTURE texture, int x, int y)
    {
        if (texture == nullptr) {
            Debug::Log("gidx.h: ERROR: GetColor - texture is nullptr");
            return Color(0, 0, 0, 0);
        }

        unsigned char r, g, b, alpha;
        texture->GetPixel(x, y, r, g, b, alpha);

        return Color(r, g, b, alpha);
    }


    // Diese Funktionen funktionieren nur mit Meshes

    inline void EntityCollisionMode(LPENTITY entity, COLLISION mode)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: EntityCollisionMode - entity is nullptr");
            return;
        }

        Mesh* mesh = (entity->IsMesh()   ? entity->AsMesh()   : nullptr);
        if (mesh == nullptr) {
            Debug::Log("gidx.h: ERROR: EntityCollisionMode - Entity is not a Mesh!");
            return;
        }

        mesh->SetCollisionMode(mode);
    }

    inline LPSURFACE EntitySurface(LPENTITY entity, unsigned int index = 0)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: EntitySurface - entity is nullptr");
            return nullptr;
        }

        Mesh* mesh = (entity->IsMesh()   ? entity->AsMesh()   : nullptr);
        if (mesh == nullptr) {
            Debug::Log("gidx.h: ERROR: EntitySurface - Entity is not a Mesh!");
            return nullptr;
        }

        return mesh->GetSurface(index);
    }

    // Schaltet den Wireframe-Modus einer Surface an oder aus.
    // true  = Linien (Wireframe)
    // false = gefüllte Dreiecke (Standard)
    inline void SurfaceWireframe(LPSURFACE surface, bool enabled)
    {
        if (!surface) { Debug::Log("gidx.h: ERROR: SurfaceWireframe - surface is nullptr"); return; }
        surface->gpu->SetWireframe(enabled);
    }

    inline bool SurfaceWireframe(LPSURFACE surface)
    {
        if (!surface) { Debug::Log("gidx.h: ERROR: SurfaceWireframe - surface is nullptr"); return false; }
        return surface->gpu->IsWireframe();
    }

    inline bool EntityCollision(LPENTITY entity1, LPENTITY entity2)
    {
        if (entity1 == nullptr || entity2 == nullptr) {
            Debug::Log("gidx.h: ERROR: EntityCollision - one or both entities are nullptr");
            return false;
        }

        Mesh* mesh1 = (entity1->IsMesh()   ? entity1->AsMesh()   : nullptr);
        Mesh* mesh2 = (entity2->IsMesh()   ? entity2->AsMesh()   : nullptr);

        if (mesh1 == nullptr || mesh2 == nullptr) {
            Debug::Log("gidx.h: ERROR: EntityCollision - one or both entities are not Meshes!");
            return false;
        }

        return mesh1->CheckCollision(mesh2);
    }

    inline DirectX::BoundingOrientedBox* EntityOBB(LPENTITY entity)
    {
        if (entity == nullptr) {
            Debug::Log("gidx.h: ERROR: EntityOBB - entity is nullptr");
            return nullptr;
        }

        Mesh* mesh = (entity->IsMesh()   ? entity->AsMesh()   : nullptr);
        if (mesh == nullptr) {
            Debug::Log("gidx.h: ERROR: EntityOBB - Entity is not a Mesh!");
            return nullptr;
        }

        return &mesh->obb;
    }

    // Aktiviert/Deaktiviert eine Entity komplett.
    // active = false: kein Update(), keine Physik, kein Rendering.
    inline void EntityActive(LPENTITY entity, bool active)
    {
        if (!entity) { Debug::Log("gidx.h: ERROR: EntityActive - entity is nullptr"); return; }
        entity->SetActive(active);
    }

    inline bool EntityActive(LPENTITY entity)
    {
        if (!entity) { Debug::Log("gidx.h: ERROR: EntityActive - entity is nullptr"); return false; }
        return entity->IsActive();
    }

    // Setzt die Sichtbarkeit einer Entity.
    // visible = false: Update() läuft weiter (Logik, Physik), aber kein Rendering.
    inline void ShowEntity(LPENTITY entity, bool visible)
    {
        if (!entity) { Debug::Log("gidx.h: ERROR: ShowEntity - entity is nullptr"); return; }
        entity->SetVisible(visible);
    }

    inline bool EntityVisible(LPENTITY entity)
    {
        if (!entity) { Debug::Log("gidx.h: ERROR: EntityVisible - entity is nullptr"); return false; }
        return entity->IsVisible();
    }

    // Setzt den Layer-Bitmask einer Entity.
    // Kombiniere mit LAYER_*-Konstanten aus RenderLayers.h
    // Beispiel: EntityLayer(mesh, LAYER_DEFAULT | LAYER_REFLECTION);
    inline void EntityLayer(LPENTITY entity, uint32_t layerMask)
    {
        if (!entity) { Debug::Log("gidx.h: ERROR: EntityLayer - entity is nullptr"); return; }
        entity->SetLayerMask(layerMask);
    }

    inline uint32_t EntityLayer(LPENTITY entity)
    {
        if (!entity) { Debug::Log("gidx.h: ERROR: EntityLayer - entity is nullptr"); return 0; }
        return entity->GetLayerMask();
    }

    // ==================== CAMERA CULLING ====================

        // Setzt die cull-Maske der Kamera.
        // Nur Objekte deren layerMask mit dieser Maske übereinstimmt werden gerendert.
        // Beispiel: CameraCullMask(cam, LAYER_ALL & ~LAYER_UI);
    inline void CameraCullMask(LPENTITY camera, uint32_t mask)
    {
        if (!camera) { Debug::Log("gidx.h: ERROR: CameraCullMask - camera is nullptr"); return; }
        Camera* cam = (camera->IsCamera() ? camera->AsCamera() : nullptr);
        if (!cam) { Debug::Log("gidx.h: ERROR: CameraCullMask - Entity ist keine Camera"); return; }
        cam->cullMask = mask;
    }

    inline uint32_t CameraCullMask(LPENTITY camera)
    {
        if (!camera) { Debug::Log("gidx.h: ERROR: CameraCullMask - camera is nullptr"); return 0; }
        Camera* cam = (camera->IsCamera() ? camera->AsCamera() : nullptr);
        if (!cam) { Debug::Log("gidx.h: ERROR: CameraCullMask - Entity ist keine Camera"); return 0; }
        return cam->cullMask;
    }

    // ── Render-Textur anlegen ─────────────────────────────────────────────────
    // Erzeugt eine RTT-Instanz und schreibt den Zeiger nach *rtt.
    // width/height: Auflösung der Textur (unabhängig vom Screen).
    // Verwendung als Textur: Engine::GetRTTTexture(rtt).
    inline void CreateRenderTexture(LPRENDERTARGET* rtt, UINT width, UINT height)
    {
        if (!engine || !rtt) return;

        RenderTextureTarget* target = new RenderTextureTarget();
        target->SetDevice(&engine->m_device);
        HRESULT hr = target->Create(
            engine->m_device.GetDevice(),
            engine->m_device.GetDeviceContext(),
            width, height);

        if (FAILED(hr))
        {
            Debug::LogError("gidx.h: CreateRenderTexture() – Create() fehlgeschlagen");
            delete target;
            *rtt = nullptr;
            return;
        }

        *rtt = target;
        Debug::Log("gidx.h: CreateRenderTexture() – ", width, "x", height, " erstellt");
    }

    // ── Render-Textur freigeben ───────────────────────────────────────────────
    // Gibt alle D3D11-Ressourcen frei und löscht die RTT-Instanz.
    inline void ReleaseRenderTexture(LPRENDERTARGET* rtt)
    {
        if (!rtt || !*rtt) return;
        (*rtt)->Release();
        delete* rtt;
        *rtt = nullptr;
    }

    // ── RTT als aktiven Render-Target setzen ─────────────────────────────────
    // Alle nachfolgenden RenderWorld()-Aufrufe rendern in diese Textur.
    // rttCamera: optionale zweite Kamera für den RTT-Pass (z.B. andere Perspektive).
    //            nullptr → aktuelle Haupt-Kamera wird verwendet.
    inline void SetRenderTarget(LPRENDERTARGET rtt, LPENTITY rttCamera = nullptr)
    {
        if (!engine || !rtt) return;
        engine->GetRM().SetRTTTarget(rtt, rttCamera);
    }

    // ── Backbuffer als Render-Target wiederherstellen ────────────────────────
    // Muss nach dem RTT-Pass aufgerufen werden, bevor der normale Frame gerendert wird.
    inline void ResetRenderTarget()
    {
        if (!engine) return;
        engine->GetRM().SetRTTTarget(nullptr, nullptr);
    }

    // ── Texture-Wrapper für den Shader-Input ─────────────────────────────────
    // Gibt den internen Texture*-Wrapper zurück.
    // Direkt verwendbar mit Engine::MaterialTexture / Engine::EntityTexture.
    inline LPTEXTURE GetRTTTexture(LPRENDERTARGET rtt)
    {
        if (!rtt) return nullptr;
        return rtt->GetTextureWrapper();
    }

    // ── Clearfarbe der RTT setzen ─────────────────────────────────────────────
    inline void SetRTTClearColor(LPRENDERTARGET rtt, float r, float g, float b, float a = 1.0f)
    {
        if (!rtt) return;
        rtt->SetClearColor(r, g, b, a);
    }
} // End of namespace Engine