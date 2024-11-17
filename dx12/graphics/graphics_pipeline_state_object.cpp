#include "dx12/graphics/graphics_pipeline_state_object.h"
#include "dx12/device.h"
#include <d3dcompiler.h>

namespace dx12::graphics {

//---------------------------------------------------------------------------------
/**
 * @brief	コマンドリストに設定する
 * @param	commandList		設定先のコマンドリスト
 */
void GraphicsPipelineStateObject::setToCommandList(CommandList& commandList) noexcept {
    // パイプラインを設定
    commandList.get()->SetPipelineState(pipelineState_.Get());

    // ルートシグネチャをセット
    commandList.get()->SetGraphicsRootSignature(rootSignature_.Get());
}

//---------------------------------------------------------------------------------
/**
 * @brief	パイプラインステートを作成する
 * @return	作成に成功した場合は true
 */
bool GraphicsPipelineStateObject::createPipelineState() noexcept {
    // とりあえずここでシェーダを作成する
    shader_ = std::make_unique<Shader>("asset/color_instance.hlsl");

    // 頂点レイアウト
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0,    DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // ラスタライザステート
    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode              = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode              = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable       = true;
    rasterizerDesc.MultisampleEnable     = false;
    rasterizerDesc.AntialiasedLineEnable = false;
    rasterizerDesc.ForcedSampleCount     = 0;
    rasterizerDesc.ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // ブレンドステート
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
        FALSE,
        FALSE,
        D3D12_BLEND_ONE,
        D3D12_BLEND_ZERO,
        D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE,
        D3D12_BLEND_ZERO,
        D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    D3D12_BLEND_DESC blendDesc       = {};
    blendDesc.AlphaToCoverageEnable  = false;
    blendDesc.IndependentBlendEnable = false;
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
        blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;
    }

    // パイプラインステート
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout                        = {inputElementDescs, _countof(inputElementDescs)};
    psoDesc.pRootSignature                     = rootSignature_.Get();
    psoDesc.VS                                 = {shader_->vertexShader()->GetBufferPointer(), shader_->vertexShader()->GetBufferSize()};
    psoDesc.PS                                 = {shader_->pixelShader()->GetBufferPointer(), shader_->pixelShader()->GetBufferSize()};
    psoDesc.RasterizerState                    = rasterizerDesc;
    psoDesc.BlendState                         = blendDesc;
    psoDesc.DepthStencilState.DepthEnable      = true;
    psoDesc.DepthStencilState.StencilEnable    = false;
    psoDesc.DepthStencilState.DepthWriteMask   = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc        = D3D12_COMPARISON_FUNC_LESS;
    psoDesc.SampleMask                         = UINT_MAX;
    psoDesc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets                   = 1;
    psoDesc.RTVFormats[0]                      = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat                          = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count                   = 1;
    auto res                                   = dx12::Device::instance().device()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState_.GetAddressOf()));
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
bool GraphicsPipelineStateObject::createRootSignature() noexcept {
    // シェーダとリソースの紐付け

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

    // 構造化バッファ( t1 )
    D3D12_DESCRIPTOR_RANGE t1            = {};
    t1.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    t1.NumDescriptors                    = 1;
    t1.BaseShaderRegister                = 1;
    t1.RegisterSpace                     = 0;
    t1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

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
    rootParameters[2].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParameters[2].DescriptorTable.pDescriptorRanges   = &t1;

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
