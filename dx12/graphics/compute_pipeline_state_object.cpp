#include "dx12/graphics/compute_pipeline_state_object.h"
#include "dx12/device.h"
#include <d3dcompiler.h>

namespace dx12::graphics {


//---------------------------------------------------------------------------------
/**
 * @brief	パイプラインステートを作成する
 * @return	作成に成功した場合は true
 */
bool ComputePipelineStateObject::createPipelineState() noexcept {
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
    // とりあえずここでシェーダを作成する
    shader_ = std::make_unique<Shader>("asset/calc.hlsl");

    // シェーダとリソースの紐付け
    D3D12_DESCRIPTOR_RANGE range1            = {};
    range1.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range1.NumDescriptors                    = 1;
    range1.BaseShaderRegister                = 0;
    range1.RegisterSpace                     = 0;
    range1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_DESCRIPTOR_RANGE range2            = {};
    range2.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    range2.NumDescriptors                    = 1;
    range2.BaseShaderRegister                = 0;
    range2.RegisterSpace                     = 0;
    range2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ルートパラメータ
    D3D12_ROOT_PARAMETER rootParameters[2]              = {};
    rootParameters[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].DescriptorTable.pDescriptorRanges   = &range1;

    rootParameters[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[1].DescriptorTable.pDescriptorRanges   = &range2;


    // ルートシグネチャ
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters             = 2;
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
