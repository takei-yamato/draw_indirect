#include "dx12/graphics/compute_pipeline_state_object.h"
#include "dx12/device.h"
#include <d3dcompiler.h>

namespace dx12::graphics {

//---------------------------------------------------------------------------------
/**
 * @brief	コマンドリストに設定する
 * @param	commandList		設定先のコマンドリスト
 */
void ComputePipelineStateObject::setToCommandList(CommandList& commandList) noexcept {
    // パイプラインを設定
    commandList.get()->SetPipelineState(pipelineState_.Get());

    // ルートシグネチャをセット
    commandList.get()->SetComputeRootSignature(rootSignature_->get());
}

//---------------------------------------------------------------------------------
/**
 * @brief	パイプラインステートを作成する
 * @return	作成に成功した場合は true
 */
bool ComputePipelineStateObject::createPipelineState() noexcept {

    // パイプラインステート
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature                    = rootSignature_->get();
    psoDesc.CS                                = {shader_->computeShader()->GetBufferPointer(), shader_->computeShader()->GetBufferSize()};
    auto res                                  = dx12::Device::instance().device()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pipelineState_.GetAddressOf()));
    if (FAILED(res)) {
        ASSERT(false, "パイプラインステートの作成に失敗");
    }

    return true;
}


}  // namespace dx12::graphics
