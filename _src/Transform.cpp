#include "Transform.h"
using namespace DirectX;

// Statische Konstanten
static const XMVECTOR forwardVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
static const XMVECTOR upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
static const XMVECTOR rightVector = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

// =========== KONSTRUKTOREN ===========
Transform::Transform() :
    position(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f)),
    rotationQuat(XMQuaternionIdentity()),
    scale(XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f)),
    matricesDirty(true),
    worldMatrix(nullptr),
    vectorsDirty(true)
{
    rotationMatrix = XMMatrixIdentity();
    translationMatrix = XMMatrixIdentity();
    scalingMatrix = XMMatrixIdentity();
    lookAt = forwardVector;
    up = upVector;
    right = rightVector;
}

Transform::Transform(XMMATRIX* world) : Transform()
{
    worldMatrix = world;
}

Transform::~Transform()
{
    // Keine dynamisch allozierten Ressourcen - worldMatrix wird extern verwaltet
}

// =========== PRIVATE HELPER ===========
void Transform::UpdateMatrices() const
{
    if (!matricesDirty) return;

    // Ändern von mutable Membern ist in const-Funktion erlaubt
    scalingMatrix = XMMatrixScalingFromVector(scale);
    rotationMatrix = XMMatrixRotationQuaternion(rotationQuat);
    translationMatrix = XMMatrixTranslationFromVector(position);

    matricesDirty = false;
    vectorsDirty = true;

    // Update externe World Matrix
    if (worldMatrix) {
        *worldMatrix = scalingMatrix * rotationMatrix * translationMatrix;
    }
}

void Transform::UpdateDirectionVectors() const
{
    if (!vectorsDirty) return;

    XMMATRIX rot = GetRotation();

    lookAt = XMVector3Normalize(XMVector3TransformNormal(forwardVector, rot));
    up = XMVector3Normalize(XMVector3TransformNormal(upVector, rot));
    right = XMVector3Normalize(XMVector3TransformNormal(rightVector, rot));

    vectorsDirty = false;
}

XMVECTOR Transform::EulerToQuaternion(float pitch, float yaw, float roll) const
{
    pitch = XMConvertToRadians(pitch);
    yaw = XMConvertToRadians(yaw);
    roll = XMConvertToRadians(roll);

    return XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
}

void Transform::QuaternionToEuler(const XMVECTOR& quat, float& pitch, float& yaw, float& roll) const
{
    float x = XMVectorGetX(quat);
    float y = XMVectorGetY(quat);
    float z = XMVectorGetZ(quat);
    float w = XMVectorGetW(quat);

    // Pitch (X-Achse)
    float sinPitch = 2.0f * (w * x + y * z);
    float cosPitch = 1.0f - 2.0f * (x * x + y * y);
    pitch = XMConvertToDegrees(atan2(sinPitch, cosPitch));

    // Yaw (Y-Achse)
    float sinYaw = 2.0f * (w * y - z * x);
    sinYaw = (sinYaw > 1.0f) ? 1.0f : ((sinYaw < -1.0f) ? -1.0f : sinYaw);
    yaw = XMConvertToDegrees(asin(sinYaw));

    // Roll (Z-Achse)
    float sinRoll = 2.0f * (w * z + x * y);
    float cosRoll = 1.0f - 2.0f * (y * y + z * z);
    roll = XMConvertToDegrees(atan2(sinRoll, cosRoll));
}

// =========== EXISTIERENDE API ===========
void Transform::Rotate(float fRotateX, float fRotateY, float fRotateZ, Space space)
{
    XMVECTOR delta = EulerToQuaternion(fRotateX, fRotateY, fRotateZ);

    if (space == Space::World) {
        rotationQuat = XMQuaternionMultiply(delta, rotationQuat);
    }
    else {
        rotationQuat = XMQuaternionMultiply(rotationQuat, delta);
    }

    rotationQuat = XMQuaternionNormalize(rotationQuat);
    matricesDirty = true;
}

void Transform::Turn(float fRotateX, float fRotateY, float fRotateZ, Space space)
{
    Rotate(fRotateX, -fRotateY, fRotateZ, space);
}

void Transform::Position(float x, float y, float z)
{
    position = XMVectorSet(x, y, z, 1.0f);
    matricesDirty = true;
}

void Transform::Move(float x, float y, float z, Space space)
{
    XMVECTOR trans = XMVectorSet(x, y, z, 0.0f);

    if (space == Space::Local) {
        trans = XMVector3Rotate(trans, rotationQuat);
    }

    position = XMVectorAdd(position, trans);
    matricesDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
    scale = XMVectorSet(x, y, z, 0.0f);
    matricesDirty = true;
}

void Transform::LookAt(const XMVECTOR& target, const XMVECTOR& upVec)
{
    XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(target, position));

    XMVECTOR up = upVec;

    // Degenerationsschutz
    float dot = fabsf(XMVectorGetX(XMVector3Dot(forward, up)));
    if (dot > 0.9999f) {
        up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        dot = fabsf(XMVectorGetX(XMVector3Dot(forward, up)));
        if (dot > 0.9999f) {
            up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        }
    }

    XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, forward));
    up = XMVector3Cross(forward, right);

    XMMATRIX rotMatrix;
    rotMatrix.r[0] = XMVectorSetW(right, 0.0f);
    rotMatrix.r[1] = XMVectorSetW(up, 0.0f);
    rotMatrix.r[2] = XMVectorSetW(forward, 0.0f);
    rotMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    rotationQuat = XMQuaternionNormalize(XMQuaternionRotationMatrix(rotMatrix));
    matricesDirty = true;
    vectorsDirty = true;  // ← WICHTIG: Vektoren müssen neu berechnet werden!
}

float Transform::Distance(const XMVECTOR& target) const
{
    XMVECTOR difference = XMVectorSubtract(target, position);
    return XMVectorGetX(XMVector3Length(difference));
}

XMMATRIX Transform::GetLocalTransformationMatrix() const
{
    UpdateMatrices();
    return scalingMatrix * rotationMatrix * translationMatrix;
}

XMVECTOR Transform::GetLookAt() const
{
    UpdateDirectionVectors();
    return lookAt;
}

XMVECTOR Transform::GetUp() const
{
    UpdateDirectionVectors();
    return up;
}

XMVECTOR Transform::GetRight() const
{
    UpdateDirectionVectors();
    return right;
}

XMMATRIX Transform::GetRotation() const
{
    UpdateMatrices();
    return rotationMatrix;
}

XMMATRIX Transform::GetTranslation() const
{
    UpdateMatrices();
    return translationMatrix;
}

XMMATRIX Transform::GetScaling() const
{
    UpdateMatrices();
    return scalingMatrix;
}

// Legacy-Methoden
float Transform::GetRoll(const XMMATRIX* XMMatrix_p_Rotation) const
{
    XMFLOAT4X4 values;
    XMStoreFloat4x4(&values, XMMatrixTranspose(*XMMatrix_p_Rotation));
    return (float)atan2(values._21, values._22);
}

float Transform::GetYaw(const XMMATRIX* XMMatrix_p_Rotation) const
{
    XMFLOAT4X4 values;
    XMStoreFloat4x4(&values, XMMatrixTranspose(*XMMatrix_p_Rotation));
    return atan2(values._13, values._33);
}

float Transform::GetPitch(const XMMATRIX* XMMatrix_p_Rotation) const
{
    XMFLOAT4X4 values;
    XMStoreFloat4x4(&values, XMMatrixTranspose(*XMMatrix_p_Rotation));
    float sinPitch = -values._23;
    float cosPitch = values._33;
    return atan2(sinPitch, cosPitch);
}

void Transform::SetRotationQuaternion(const XMVECTOR& quaternion)
{
    rotationQuat = XMQuaternionNormalize(quaternion);
    matricesDirty = true;
}

void Transform::RotateQuaternion(const XMVECTOR& quaternion, Space space)
{
    XMVECTOR q = XMQuaternionNormalize(quaternion);

    if (space == Space::World) {
        rotationQuat = XMQuaternionMultiply(q, rotationQuat);
    }
    else {
        rotationQuat = XMQuaternionMultiply(rotationQuat, q);
    }

    rotationQuat = XMQuaternionNormalize(rotationQuat);
    matricesDirty = true;
}

// 2. EULER-WINKEL GETTER
float Transform::GetPitch() const
{
    float pitch, yaw, roll;
    QuaternionToEuler(rotationQuat, pitch, yaw, roll);
    return pitch;
}

float Transform::GetYaw() const
{
    float pitch, yaw, roll;
    QuaternionToEuler(rotationQuat, pitch, yaw, roll);
    return yaw;
}

float Transform::GetRoll() const
{
    float pitch, yaw, roll;
    QuaternionToEuler(rotationQuat, pitch, yaw, roll);
    return roll;
}

// 3. SKALIERUNG GETTER/SETTER
void Transform::SetScale(float x, float y, float z)
{
    scale = XMVectorSet(x, y, z, 0.0f);
    matricesDirty = true;
}

void Transform::SetScale(float uniformScale)
{
    scale = XMVectorSet(uniformScale, uniformScale, uniformScale, 0.0f);
    matricesDirty = true;
}

// 4. TRANSFORM OPERATIONEN
void Transform::Translate(const XMVECTOR& translation, Space space)
{
    if (space == Space::Local) {
        position = XMVectorAdd(position, XMVector3Rotate(translation, rotationQuat));
    }
    else {
        position = XMVectorAdd(position, translation);
    }
    matricesDirty = true;
}

void Transform::SetPosition(const XMVECTOR& pos)
{
    position = XMVectorSetW(pos, 1.0f);
    matricesDirty = true;
}

// 5. INTERPOLATION
void Transform::Lerp(const Transform& target, float t)
{
    // Linear interpolation für Position, Scale und Rotation
    position = XMVectorLerp(position, target.position, t);
    scale = XMVectorLerp(scale, target.scale, t);
    rotationQuat = XMQuaternionSlerp(rotationQuat, target.rotationQuat, t);
    matricesDirty = true;
}

void Transform::Slerp(const Transform& target, float t)
{
    // Spherical linear interpolation für alle (Position, Scale und Rotation)
    // Position: Slerp für gleichmäßigere Bewegung
    XMVECTOR posLength = XMVector3Length(XMVectorSubtract(target.position, position));
    float distance = XMVectorGetX(posLength);

    if (distance > 0.0001f) {
        XMVECTOR direction = XMVector3Normalize(XMVectorSubtract(target.position, position));
        position = XMVectorAdd(position, XMVectorScale(direction, distance * t));
    }
    else {
        position = XMVectorLerp(position, target.position, t);
    }

    // Scale: Slerp für konsistente Skalierung
    XMFLOAT3 currentScale, targetScale;
    XMStoreFloat3(&currentScale, scale);
    XMStoreFloat3(&targetScale, target.scale);

    float scaleX = currentScale.x + (targetScale.x - currentScale.x) * t;
    float scaleY = currentScale.y + (targetScale.y - currentScale.y) * t;
    float scaleZ = currentScale.z + (targetScale.z - currentScale.z) * t;
    scale = XMVectorSet(scaleX, scaleY, scaleZ, 0.0f);

    // Rotation: Slerp
    rotationQuat = XMQuaternionSlerp(rotationQuat, target.rotationQuat, t);
    matricesDirty = true;
}

// 6. HELPER
XMMATRIX Transform::GetWorldMatrix() const
{
    UpdateMatrices();
    return scalingMatrix * rotationMatrix * translationMatrix;
}

// 7. TRANSFORM COMBINATIONS
Transform Transform::Combine(const Transform& other) const
{
    Transform result;

    result.position = XMVectorAdd(position,
        XMVector3Rotate(other.position, rotationQuat));
    result.rotationQuat = XMQuaternionMultiply(rotationQuat, other.rotationQuat);

    XMFLOAT3 thisScale, otherScale;
    XMStoreFloat3(&thisScale, scale);
    XMStoreFloat3(&otherScale, other.scale);

    result.scale = XMVectorSet(
        thisScale.x * otherScale.x,
        thisScale.y * otherScale.y,
        thisScale.z * otherScale.z,
        0.0f
    );

    result.matricesDirty = true;
    return result;
}

Transform Transform::Inverse() const
{
    Transform result;

    result.position = XMVector3Rotate(XMVectorNegate(position),
        XMQuaternionConjugate(rotationQuat));
    result.rotationQuat = XMQuaternionConjugate(rotationQuat);

    XMFLOAT3 s;
    XMStoreFloat3(&s, scale);

    // Validierung gegen Division durch Zero
    result.scale = XMVectorSet(
        (s.x != 0.0f) ? 1.0f / s.x : 1.0f,
        (s.y != 0.0f) ? 1.0f / s.y : 1.0f,
        (s.z != 0.0f) ? 1.0f / s.z : 1.0f,
        0.0f
    );

    result.matricesDirty = true;
    return result;
}

// 8. CONSTANT BUFFER DATA
Transform::TransformData Transform::GetTransformData() const
{
    TransformData data;

    XMMATRIX world = GetWorldMatrix();
    XMMATRIX worldInverseTranspose = XMMatrixInverse(nullptr,
        XMMatrixTranspose(world));

    XMStoreFloat4x4(&data.worldMatrix, world);
    XMStoreFloat4x4(&data.worldInverseTransposeMatrix, worldInverseTranspose);

    return data;
}