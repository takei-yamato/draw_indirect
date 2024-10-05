#pragma once

#include "dx12/device.h"

namespace camera {

// 平面
struct Plane {
    DirectX::XMVECTOR normal;  // 平面の法線ベクトル
    float             d;       // 原点からの距離
};

// 視錐台
struct Frustum {
    Plane planes[4];  // 近遠を省略した視錐台（四角錐）を構成する平面
};

//---------------------------------------------------------------------------------
/**
 * @brief	ビュープロジェクションから視錐台を作成する
 * @param	viewProj	ビュープロジェクション行列
 * @retval	生成された視錐台
 */
Frustum createFrustumFromViewProjection(const DirectX::XMMATRIX& viewProj);

//---------------------------------------------------------------------------------
/**
 * @brief	指定した位置が視錐台に含まれるかチェックする
 * @param	frustum		視錐台
 * @param	pos			位置
 * @retval	含まれる場合は true
 */
bool isPositionInFrustum(const Frustum& frustum, const DirectX::XMFLOAT3& pos);

}  // namespace utility