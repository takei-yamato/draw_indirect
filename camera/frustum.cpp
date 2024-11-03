#include "def.h"
#include "camera/frustum.h"

namespace camera {

//---------------------------------------------------------------------------------
/**
 * @brief	ビュープロジェクションから視錐台を作成する
 * @param	viewProj	ビュープロジェクション行列
 * @retval	生成された視錐台
 */
Frustum createFrustumFromViewProjection(const DirectX::XMMATRIX& viewProj) {
    Frustum frustum{};

    DirectX::XMFLOAT4 v0, v1, v2, v3;
    DirectX::XMStoreFloat4(&v0, viewProj.r[0]);
    DirectX::XMStoreFloat4(&v1, viewProj.r[1]);
    DirectX::XMStoreFloat4(&v2, viewProj.r[2]);
    DirectX::XMStoreFloat4(&v3, viewProj.r[3]);

    // 視錐台の平面をビュープロジェクション行列から抽出
    // 左平面
    frustum.planes[0].normal = DirectX::XMVectorSet(v0.w + v0.x,
                                                    v1.w + v1.x,
                                                    v2.w + v2.x, 0.0f);
    frustum.planes[0].d      = DirectX::XMVectorSet(v3.w + v3.x, 0, 0, 0);

    // 右平面
    frustum.planes[1].normal = DirectX::XMVectorSet(v0.w - v0.x,
                                                    v1.w - v1.x,
                                                    v2.w - v2.x, 0.0f);
    frustum.planes[1].d      = DirectX::XMVectorSet(v3.w - v3.x, 0, 0, 0);

    // 下平面
    frustum.planes[2].normal = DirectX::XMVectorSet(v0.w + v0.y,
                                                    v1.w + v1.y,
                                                    v2.w + v2.y, 0.0f);
    frustum.planes[2].d      = DirectX::XMVectorSet(v3.w + v3.y, 0, 0, 0);

    // 上平面
    frustum.planes[3].normal = DirectX::XMVectorSet(v0.w - v0.y,
                                                    v1.w - v1.y,
                                                    v2.w - v2.y, 0.0f);
    frustum.planes[3].d      = DirectX::XMVectorSet(v3.w - v3.y, 0, 0, 0);

    for (int i = 0; i < 4; ++i) {
        // 平面法線
        DirectX::XMVECTOR normal = frustum.planes[i].normal;
        float             length = DirectX::XMVectorGetX(DirectX::XMVector3Length(normal));
        auto              lvec   = DirectX::XMVectorReplicate(length);
        frustum.planes[i].normal = DirectX::XMVectorDivide(normal, lvec);

        // 原点から平面までの最短距離
        // 平面の法線だけでは「原点を含む平面」として判定してしまう為、
        // 「視点を含む平面」として判定するために距離が必要になる
        frustum.planes[i].d = DirectX::XMVectorDivide(frustum.planes[i].d, lvec);
    }

    return frustum;
}

//---------------------------------------------------------------------------------
/**
 * @brief	指定した位置が視錐台に含まれるかチェックする
 * @param	frustum		視錐台
 * @param	pos			位置
 * @retval	含まれる場合は true
 */
bool isPositionInFrustum(const Frustum& frustum, const DirectX::XMFLOAT3& pos) {
    for (const auto& plane : frustum.planes) {
        float distance = DirectX::XMVectorGetX(DirectX::XMVectorAdd(DirectX::XMVector3Dot(plane.normal, DirectX::XMLoadFloat3(&pos)), plane.d));
        if (distance < 0) {
            return false;  // 外にある
        }
    }
    return true;  // 中にある
}

}  // namespace camera
