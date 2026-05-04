#include "command_signature.h"

namespace dx12::graphics {

//---------------------------------------------------------------------------------
/**
 * @brief	コマンドシグネチャを作成する
 * @param	rootSignature	ルートシグネチャ
 * @param	commandSize		コマンド構造体のサイズ
 * @return	作成に成功した場合は true
 */
bool CommandSignature::create(const RootSignature* rootSignature, uint32_t commandSize) noexcept {
    // コマンドシグネチャの引数フォーマットを定義
    // IndirectArgs 構造体の定義に合わせて、SRV と DrawIndexedInstanced の引数を指定
    D3D12_INDIRECT_ARGUMENT_DESC argDesc[2]          = {};
    argDesc[0].Type                                  = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;  // SRV
    argDesc[0].ShaderResourceView.RootParameterIndex = 2;                                                  // SRV のスロット番号
    argDesc[1].Type                                  = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;          // DrawIndexedInstanced の引数

    // コマンドシグネチャを作成
    D3D12_COMMAND_SIGNATURE_DESC csDesc{};
    csDesc.pArgumentDescs   = argDesc;
    csDesc.NumArgumentDescs = 2;
    csDesc.ByteStride       = commandSize;
    csDesc.NodeMask         = 0;
    auto res                = Device::instance().device()->CreateCommandSignature(&csDesc, rootSignature->get(), IID_PPV_ARGS(commandSignature_.GetAddressOf()));
    ASSERT(SUCCEEDED(res), "コマンドシグネチャの作成に失敗");

    return SUCCEEDED(res);
}
}  // namespace dx12::graphics
