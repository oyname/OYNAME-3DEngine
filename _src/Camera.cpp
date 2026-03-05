#include "Camera.h"
using namespace DirectX;

Camera::Camera() : Entity(EntityType::Camera)
{
    // Matrizen werden in Entity::Entity() initialisiert
}

Camera::~Camera()
{
    // Cleanup in Entity::~Entity()
}

void Camera::UpdateCamera(XMVECTOR position, XMVECTOR direction, XMVECTOR up)
{
    // direction ist eine RICHTUNG, berechne target
    XMVECTOR target = DirectX::XMVectorAdd(position, direction);

    // Nutze die geerbte Entity-Methode
    GenerateViewMatrix(position, target, up);
}
