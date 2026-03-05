#pragma once
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "gdxutil.h"

enum class Space {
    Local,
    World
};

class Transform
{
private:
    // INTERNE SPEICHERUNG MIT QUATERNIONEN
    DirectX::XMVECTOR position;          // W=1.0f
    DirectX::XMVECTOR rotationQuat;      // Quaternion (X, Y, Z, W)
    DirectX::XMVECTOR scale;             // W=0.0f

    // GECACHTE MATRIZEN (mutable für const Getter)
    mutable DirectX::XMMATRIX rotationMatrix;
    mutable DirectX::XMMATRIX translationMatrix;
    mutable DirectX::XMMATRIX scalingMatrix;
    mutable bool matricesDirty;

    // EXTERNE REFERENZ (nicht mutable - nur zum Schreiben)
    DirectX::XMMATRIX* worldMatrix;

    // CACHED DIRECTION VECTORS (mutable für const Getter)
    mutable DirectX::XMVECTOR lookAt;
    mutable DirectX::XMVECTOR up;
    mutable DirectX::XMVECTOR right;
    mutable bool vectorsDirty;

    // PRIVATE METHODEN
    void UpdateMatrices() const;
    void UpdateDirectionVectors() const;
    DirectX::XMVECTOR EulerToQuaternion(float pitch, float yaw, float roll) const;
    void QuaternionToEuler(const DirectX::XMVECTOR& quat, float& pitch, float& yaw, float& roll) const;

public:
    // =========== KONSTRUKTOREN ===========
    Transform();
    explicit Transform(DirectX::XMMATRIX* world);
    ~Transform();

    // =========== EXISTIERENDE API ===========
    void SetWorldMatrix(DirectX::XMMATRIX* world) { worldMatrix = world; matricesDirty = true; }

    DirectX::XMMATRIX GetLocalTransformationMatrix() const;
    DirectX::XMMATRIX* GetWorldTransformationMatrix() const { return worldMatrix; }
    DirectX::XMVECTOR GetPosition() const { return position; }
    DirectX::XMVECTOR GetLookAt() const;
    DirectX::XMVECTOR GetUp() const;
    DirectX::XMVECTOR GetRight() const;
    DirectX::XMMATRIX GetRotation() const;
    DirectX::XMMATRIX GetTranslation() const;
    DirectX::XMMATRIX GetScaling() const;

    void Rotate(float fRotateX, float fRotateY, float fRotateZ, Space space = Space::Local);
    void Turn(float fRotateX, float fRotateY, float fRotateZ, Space space = Space::Local);
    void Position(float x, float y, float z);
    void Move(float x, float y, float z, Space space = Space::Local);
    void Scale(float x, float y, float z);
    void LookAt(const DirectX::XMVECTOR& target, const DirectX::XMVECTOR& upVector);
    float Distance(const DirectX::XMVECTOR& target) const;

    // =========== NEUE FEATURES ===========

    // 1. QUATERNION-METHODEN
    DirectX::XMVECTOR GetRotationQuaternion() const { return rotationQuat; }
    void SetRotationQuaternion(const DirectX::XMVECTOR& quaternion);
    void RotateQuaternion(const DirectX::XMVECTOR& quaternion, Space space = Space::Local);

    // 2. EULER-WINKEL GETTER
    float GetPitch() const;     // In Grad
    float GetYaw() const;       // In Grad
    float GetRoll() const;      // In Grad

    // 3. SKALIERUNG GETTER/SETTER
    DirectX::XMVECTOR GetScaleVector() const { return scale; }
    void SetScale(float x, float y, float z);
    void SetScale(float uniformScale);

    // 4. TRANSFORM OPERATIONEN
    void Translate(const DirectX::XMVECTOR& translation, Space space = Space::Local);
    void SetPosition(const DirectX::XMVECTOR& pos);

    // 5. INTERPOLATION
    void Lerp(const Transform& target, float t);
    void Slerp(const Transform& target, float t);

    // 6. HELPER
    bool HasChanged() const { return matricesDirty; }
    void SetChanged(bool changed = true) { matricesDirty = changed; if (changed) vectorsDirty = true; }
    DirectX::XMMATRIX GetWorldMatrix() const;

    // 7. TRANSFORM COMBINATIONS
    Transform Combine(const Transform& other) const;
    Transform Inverse() const;

    // 8. CONSTANT BUFFER DATA
    struct TransformData {
        DirectX::XMFLOAT4X4 worldMatrix;
        DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;
    };
    TransformData GetTransformData() const;

    // Legacy-Methoden
    float GetRoll(const DirectX::XMMATRIX* XMMatrix_p_Rotation) const;
    float GetYaw(const DirectX::XMMATRIX* XMMatrix_p_Rotation) const;
    float GetPitch(const DirectX::XMMATRIX* XMMatrix_p_Rotation) const;
};