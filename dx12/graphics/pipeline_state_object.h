#pragma once

#include "dx12/command_list.h"
#include "dx12/graphics/shader.h"

#include "utility/noncopyable.h"

namespace dx12::graphics {
//---------------------------------------------------------------------------------
/**
 * @brief
 * パイプラインステートオブジェクト
 */
class PipelineStateObject : public utility::Noncopyable {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    PipelineStateObject() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    virtual ~PipelineStateObject() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	パイプラインステートオブジェクトを作成する
     * @return	作成に成功した場合は true
     */
    bool create() noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストに設定する
     * @param	commandList		設定先のコマンドリスト
     */
    void setToCommandList(CommandList& commandList) noexcept;

protected:
    //---------------------------------------------------------------------------------
    /**
     * @brief	パイプラインステートを作成する
     * @return	作成に成功した場合は true
     */
    virtual bool createPipelineState() noexcept = 0;

    //---------------------------------------------------------------------------------
    /**
     * @brief	ルートシグネチャを作成する
     * @return	作成に成功した場合は true
     */
    virtual bool createRootSignature() noexcept = 0;

protected:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_{};  ///< パイプラインステート
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_{};  ///< ルートシグネチャ
    std::unique_ptr<Shader>                     shader_{};         ///< シェーダ
};
}  // namespace dx12::graphics
