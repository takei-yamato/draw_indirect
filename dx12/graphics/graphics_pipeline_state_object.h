#pragma once

#include "dx12/graphics/pipeline_state_object.h"

namespace dx12::graphics {
//---------------------------------------------------------------------------------
/**
 * @brief
 * パイプラインステートオブジェクト
 */
class GraphicsPipelineStateObject final : public PipelineStateObject {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    GraphicsPipelineStateObject() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    virtual ~GraphicsPipelineStateObject() = default;

	//---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストに設定する
     * @param	commandList		設定先のコマンドリスト
     */
    void setToCommandList(CommandList& commandList) noexcept override;

protected:
    //---------------------------------------------------------------------------------
    /**
     * @brief	パイプラインステートを作成する
     * @return	作成に成功した場合は true
     */
    bool createPipelineState() noexcept override;

    //---------------------------------------------------------------------------------
    /**
     * @brief	ルートシグネチャを作成する
     * @return	作成に成功した場合は true
     */
    bool createRootSignature() noexcept override;

};
}  // namespace dx12::graphics
