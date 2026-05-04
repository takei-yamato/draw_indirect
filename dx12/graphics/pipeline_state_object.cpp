#include "dx12/graphics/pipeline_state_object.h"

namespace dx12::graphics {

//---------------------------------------------------------------------------------
/**
 * @brief	パイプラインステートオブジェクトを作成する
 * @param	rootSignature	ルートシグネチャ
 * @param	shader			シェーダ
 * @return	作成に成功した場合は true
 */
bool PipelineStateObject::create(const RootSignature* rootSignature, const Shader* shader) noexcept {
	rootSignature_ = rootSignature;
	shader_        = shader;
    return createPipelineState();
}

}  // namespace dx12::graphics
