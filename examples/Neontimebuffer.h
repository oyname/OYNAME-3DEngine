#pragma once

#include <d3d11.h>

// Constant Buffer Struct fuer den Neon-Plasma-Shader (Register b1)
// Groesse muss Vielfaches von 16 Byte sein
struct __declspec(align(16)) TimeBufferData
{
    float time;
    float padding[3];
};

// Verwaltet den Time-Constant-Buffer fuer den Neon-Plasma-Pixel-Shader.
// Holt Device und Context selbst ueber CGIDX::GetInstance(),
// konsistent mit dem Rest der Engine (z.B. LightManager).
class NeonTimeBuffer
{
public:
    NeonTimeBuffer();
    ~NeonTimeBuffer();

    // Einmalig nach Engine::Graphics() aufrufen.
    // device und context direkt aus game.cpp uebergeben:
    //   engine->m_device.GetDevice()
    //   engine->m_device.GetDeviceContext()
    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

    // Jeden Frame mit deltaTime aus Core::GetDeltaTime() aufrufen
    void Update(float deltaTime);

    // Vor Engine::RenderWorld() aufrufen:
    // Bindet den Buffer an Pixel-Shader-Register b1
    void Bind();

    // Wird vom Destruktor automatisch aufgerufen
    void Shutdown();

private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    ID3D11Buffer* m_buffer;
    float                   m_elapsedTime;
};