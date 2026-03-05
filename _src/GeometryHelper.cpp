#include "GeometryHelper.h"
#include "Surface.h"
#include <cfloat>

using namespace DirectX;

void GeometryHelper::CalculateSize(
    const Surface&  surface,
    XMMATRIX        rotationMatrix,
    XMFLOAT3&       minOut,
    XMFLOAT3&       maxOut)
{
    XMFLOAT3 minPoint = { FLT_MAX,  FLT_MAX,  FLT_MAX };
    XMFLOAT3 maxPoint = {-FLT_MAX, -FLT_MAX, -FLT_MAX };

    const auto& positions = surface.GetPositions();
    for (const auto& vertex : positions)
    {
        XMVECTOR rotated = XMVector3Transform(XMLoadFloat3(&vertex), rotationMatrix);
        XMFLOAT3 t;
        XMStoreFloat3(&t, rotated);

        if (t.x < minPoint.x) minPoint.x = t.x;
        if (t.y < minPoint.y) minPoint.y = t.y;
        if (t.z < minPoint.z) minPoint.z = t.z;
        if (t.x > maxPoint.x) maxPoint.x = t.x;
        if (t.y > maxPoint.y) maxPoint.y = t.y;
        if (t.z > maxPoint.z) maxPoint.z = t.z;
    }

    minOut = minPoint;
    maxOut = maxPoint;
}
