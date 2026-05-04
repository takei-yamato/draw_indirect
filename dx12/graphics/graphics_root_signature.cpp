#include "dx12/graphics/graphics_root_signature.h"

namespace dx12::graphics {

//---------------------------------------------------------------------------------
/**
 * @brief	ルートシグネチャを作成する
 * @return	作成に成功した場合は true
 */
bool GraphicsRootSignature::create() noexcept {

    // コンスタントバッファ( b0 )
    D3D12_DESCRIPTOR_RANGE b            = {};
    b.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    b.NumDescriptors                    = 1;
    b.BaseShaderRegister                = 0;
    b.RegisterSpace                     = 0;
    b.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // 構造化バッファ( t0 )
    D3D12_DESCRIPTOR_RANGE t0            = {};
    t0.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    t0.NumDescriptors                    = 1;
    t0.BaseShaderRegister                = 0;
    t0.RegisterSpace                     = 0;
    t0.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ディスクリプタテーブルではなく直接 SRV をルートパラメータで指定する
    //// 構造化バッファ( t1 )
    // D3D12_DESCRIPTOR_RANGE t1            = {};
    // t1.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    // t1.NumDescriptors                    = 1;
    // t1.BaseShaderRegister                = 1;
    // t1.RegisterSpace                     = 0;
    // t1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // スタティックサンプラ( s0 )
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter                    = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW                  = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.MipLODBias                = 0;
    sampler.MaxAnisotropy             = 0;
    sampler.ComparisonFunc            = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor               = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD                    = 0.0f;
    sampler.MaxLOD                    = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister            = 0;
    sampler.RegisterSpace             = 0;
    sampler.ShaderVisibility          = D3D12_SHADER_VISIBILITY_ALL;

    // ルートパラメータ
    constexpr auto       paramNum                 = 3;
    D3D12_ROOT_PARAMETER rootParameters[paramNum] = {};

    rootParameters[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[0].DescriptorTable.pDescriptorRanges   = &b;
    rootParameters[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[1].DescriptorTable.pDescriptorRanges   = &t0;
    // rootParameters[2].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    // rootParameters[2].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_VERTEX;
    // rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    // rootParameters[2].DescriptorTable.pDescriptorRanges   = &t1;
    rootParameters[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParameters[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[2].Descriptor.ShaderRegister = 1;
    rootParameters[2].Descriptor.RegisterSpace  = 0;

    // ルートシグネチャ
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters             = paramNum;
    rootSignatureDesc.pParameters               = rootParameters;
    rootSignatureDesc.NumStaticSamplers         = 1;
    rootSignatureDesc.pStaticSamplers           = &sampler;
    rootSignatureDesc.Flags                     = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> error;

    auto res = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf());
    if (FAILED(res)) {
        char* p = static_cast<char*>(error->GetBufferPointer());
        ASSERT(false, p);
        return false;
    }

    res = dx12::Device::instance().device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature_.GetAddressOf()));
    if (FAILED(res)) {
        ASSERT(false, "ルートシグネチャの生成に失敗");
        return false;
    }

    return true;
}

}  // namespace dx12::graphics
