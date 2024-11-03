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
    commandList.get()->SetComputeRootSignature(rootSignature_.Get());
}

//---------------------------------------------------------------------------------
/**
 * @brief	パイプラインステートを作成する
 * @return	作成に成功した場合は true
 */
bool ComputePipelineStateObject::createPipelineState() noexcept {
    // とりあえずここでシェーダを作成する
    shader_ = std::make_unique<Shader>("asset/calc.hlsl");

    // パイプラインステート
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature                    = rootSignature_.Get();
    psoDesc.CS                                = {shader_->computeShader()->GetBufferPointer(), shader_->computeShader()->GetBufferSize()};
    auto res                                  = dx12::Device::instance().device()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pipelineState_.GetAddressOf()));
    if (FAILED(res)) {
        ASSERT(false, "パイプラインステートの作成に失敗");
    }

    return true;
}

//---------------------------------------------------------------------------------
/**
 * @brief	ルートシグネチャを作成する
 * @return	作成に成功した場合は true
 */
bool ComputePipelineStateObject::createRootSignature() noexcept {
    // コンスタントバッファ( b0 )
    D3D12_DESCRIPTOR_RANGE b0            = {};
    b0.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    b0.NumDescriptors                    = 1;
    b0.BaseShaderRegister                = 0;
    b0.RegisterSpace                     = 0;
    b0.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // コンスタントバッファ( b1 )
    D3D12_DESCRIPTOR_RANGE b1            = {};
    b1.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    b1.NumDescriptors                    = 1;
    b1.BaseShaderRegister                = 1;
    b1.RegisterSpace                     = 0;
    b1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // 構造化バッファ（t0）
    D3D12_DESCRIPTOR_RANGE t            = {};
    t.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    t.NumDescriptors                    = 1;
    t.BaseShaderRegister                = 0; 
    t.RegisterSpace                     = 0;
    t.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // アンオーダードバッファ（u0）
    D3D12_DESCRIPTOR_RANGE u            = {};
    u.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    u.NumDescriptors                    = 1;
    u.BaseShaderRegister                = 0;
    u.RegisterSpace                     = 0;
    u.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ルートパラメータ
    constexpr auto       paramNum                 = 4;
    D3D12_ROOT_PARAMETER rootParameters[paramNum] = {};

    rootParameters[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].DescriptorTable.pDescriptorRanges   = &b0;
    rootParameters[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[1].DescriptorTable.pDescriptorRanges   = &b1;
    rootParameters[2].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[2].DescriptorTable.pDescriptorRanges   = &t;
    rootParameters[3].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[3].DescriptorTable.pDescriptorRanges   = &u;

    // ルートシグネチャ
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters             = paramNum;
    rootSignatureDesc.pParameters               = rootParameters;
    rootSignatureDesc.NumStaticSamplers         = 0;
    rootSignatureDesc.pStaticSamplers           = {};
    rootSignatureDesc.Flags                     = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;

    auto res = D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        signature.GetAddressOf(),
        error.GetAddressOf());
    if (FAILED(res)) {
        ASSERT(false, "ルートシグネチャのシリアライズに失敗");
        return false;
    }

    res = dx12::Device::instance().device()->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(rootSignature_.GetAddressOf()));
    if (FAILED(res)) {
        ASSERT(false, "ルートシグネチャの生成に失敗");
        return false;
    }

    return true;
}

}  // namespace dx12::graphics
