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

}  // namespace dx12::graphics
