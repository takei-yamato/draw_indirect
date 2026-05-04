#pragma once

#include "root_signature.h"

namespace dx12::graphics {
//---------------------------------------------------------------------------------
/**
 * @brief
 * ルートシグネチャ
 */
class GraphicsRootSignature : public RootSignature {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    GraphicsRootSignature() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    virtual ~GraphicsRootSignature() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	ルートシグネチャを作成する
     * @return	作成に成功した場合は true
     */
    virtual bool create() noexcept override;
};
}  // namespace dx12::graphics
