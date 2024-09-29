#include "dx12/graphics/pipeline_state_object.h"

namespace dx12::graphics {

//---------------------------------------------------------------------------------
/**
 * @brief	パイプラインステートオブジェクトを作成する
 * @return	作成に成功した場合は true
 */
bool PipelineStateObject::create() noexcept {
    auto res = true;

    res &= createRootSignature();
    res &= createPipelineState();

    return res;
}

//---------------------------------------------------------------------------------
/**
 * @brief	コマンドリストに設定する
 * @param	commandList		設定先のコマンドリスト
 */
void PipelineStateObject::setToCommandList(CommandList& commandList) noexcept {

    // パイプラインを設定
    commandList.get()->SetPipelineState(pipelineState_.Get());

    // ルートシグネチャをセット
    commandList.get()->SetGraphicsRootSignature(rootSignature_.Get());
}


}  // namespace dx12::graphics
