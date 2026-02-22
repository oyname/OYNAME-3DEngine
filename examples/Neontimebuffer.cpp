#include "NeonTimeBuffer.h"
#include "gidx.h"

NeonTimeBuffer::NeonTimeBuffer()
    : m_device(nullptr)
    , m_context(nullptr)
    , m_buffer(nullptr)
    , m_elapsedTime(0.0f)
{
}

NeonTimeBuffer::~NeonTimeBuffer()
{
    Shutdown();
}

bool NeonTimeBuffer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    if (!device || !context)
    {
        Debug::Log("NeonTimeBuffer.cpp: Initialize - Device oder Context ist nullptr. Engine::Graphics() zuerst aufrufen.");
        return false;
    }

    m_device = device;
    m_context = context;

    // DYNAMIC Buffer wie alle Constant Buffer in dieser Engine
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(TimeBufferData);  // 16 Byte
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;

    TimeBufferData initData = {};
    initData.time = 0.0f;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = &initData;

    HRESULT hr = m_device->CreateBuffer(&desc, &sd, &m_buffer);
    if (FAILED(hr))
    {
        Debug::Log("NeonTimeBuffer.cpp: Initialize - CreateBuffer fehlgeschlagen.");
        return false;
    }

    Debug::Log("NeonTimeBuffer.cpp: Initialize - TimeBuffer erfolgreich erstellt (PS-Register b1).");
    return true;
}

void NeonTimeBuffer::Update(float deltaTime)
{
    m_elapsedTime += deltaTime;

    if (!m_buffer || !m_context)
        return;

    // DYNAMIC Buffer: immer Map/Unmap, niemals UpdateSubresource
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT hr = m_context->Map(m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        Debug::Log("NeonTimeBuffer.cpp: Update - Map fehlgeschlagen.");
        return;
    }

    TimeBufferData* data = reinterpret_cast<TimeBufferData*>(mapped.pData);
    data->time = m_elapsedTime;
    data->padding[0] = 0.0f;
    data->padding[1] = 0.0f;
    data->padding[2] = 0.0f;

    m_context->Unmap(m_buffer, 0);
}

void NeonTimeBuffer::Bind()
{
    if (!m_buffer || !m_context)
    {
        Debug::Log("NeonTimeBuffer.cpp: Bind - Buffer oder Context ist nullptr.");
        return;
    }

    // Belegte PS-Slots der Engine:
    // b0 = MatrixBuffer   (Entity.cpp)
    // b1 = LightBuffer    (LightManager.cpp)
    // b2 = MaterialBuffer (Material.cpp)
    // b3 = frei (ShadowMatrix nur im VS)
    // b4 = TimeBuffer     (unser Neon-Effekt)
    m_context->PSSetConstantBuffers(4, 1, &m_buffer);
}

void NeonTimeBuffer::Shutdown()
{
    if (m_buffer)
    {
        m_buffer->Release();
        m_buffer = nullptr;
        Debug::Log("NeonTimeBuffer.cpp: Shutdown - TimeBuffer freigegeben.");
    }

    m_device = nullptr;
    m_context = nullptr;
}