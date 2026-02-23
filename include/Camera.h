#pragma once
#include <cstdint>
#include "Entity.h"
#include "RenderLayers.h"
#include "gdxutil.h"

class Camera : public Entity
{
public:
    enum PROJ_MODE
    {
        ORTHO_LH,
        PERSPECTIVE_LH,
    };

    struct Perspective
    {
        float fieldOfView;
        float aspectRatio;
        float nearPlane;
        float farPlane;
    };

    // cullMask: welche Layers sieht diese Kamera?
    // Default: alles (LAYER_ALL)
    uint32_t cullMask = LAYER_ALL;

public:
    Camera();
    ~Camera();

    // UpdateCamera ist Camera-spezifisch
    void UpdateCamera(DirectX::XMVECTOR position,
                      DirectX::XMVECTOR direction,
                      DirectX::XMVECTOR up);
};

typedef Camera* LPCAMERA;
