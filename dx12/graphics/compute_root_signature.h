#pragma once

#include "dx12/graphics/root_signature.h"

namespace dx12::graphics {
//---------------------------------------------------------------------------------
/**
 * @brief
 * ルートシグネチャ
 */
class ComputeRootSignature : public RootSignature {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    ComputeRootSignature() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    virtual ~ComputeRootSignature() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	ルートシグネチャを作成する
     * @return	作成に成功した場合は true
     */
    virtual bool create() noexcept override;

};
}  // namespace dx12::graphics
