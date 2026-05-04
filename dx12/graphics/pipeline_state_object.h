#pragma once

#include "dx12/command_list.h"
#include "dx12/graphics/shader.h"
#include "dx12/graphics/root_signature.h"

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
	 * @param	rootSignature	ルートシグネチャ
	 * @param	shader			シェーダ
     * @return	作成に成功した場合は true
     */
    bool create(const RootSignature* rootSignature, const Shader* shader) noexcept;

public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストに設定する
     * @param	commandList		設定先のコマンドリスト
     */
    virtual void setToCommandList(CommandList& commandList) noexcept = 0;

protected:
    //---------------------------------------------------------------------------------
    /**
     * @brief	パイプラインステートを作成する
     * @return	作成に成功した場合は true
     */
    virtual bool createPipelineState() noexcept = 0;

protected:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_{};  ///< パイプラインステート
    const Shader*                               shader_{};         ///< シェーダの参照
    const RootSignature*                        rootSignature_{};  ///< ルートシグネチャの参照
};
}  // namespace dx12::graphics
