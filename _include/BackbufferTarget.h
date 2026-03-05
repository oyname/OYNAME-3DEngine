#pragma once
#include "IRenderTarget.h"

class BackbufferTarget : public IRenderTarget
{
public:
    // Clearfarbe setzen (RGBA, Default: schwarz)
    void SetClearColor(float r, float g, float b, float a = 1.0f);

    const float* GetClearColor() const { return m_clearColor; }

private:
    float          m_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
};
