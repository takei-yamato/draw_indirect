#pragma once

#include "dx12/device.h"
#include "dx12/command_list.h"
#include "dx12/gpu_resource.h"

#include "utility/noncopyable.h"

#include "dx12/resource/shader_resource.h"

namespace dx12::resource {

//---------------------------------------------------------------------------------
/**
 * @brief
 * アンオーダードアクセスリソース
 */
class UnorderedAccessResource final : public ResourceBase {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	リソースを生成する
     * @param	data		データの先頭アドレス
     * @param	stride		バッファのストライド
     * @param	num			バッファの数
     * @return	作成に成功した場合は true
     */
    bool create(void** data, uint32_t stride, uint32_t num) noexcept override;
};

//---------------------------------------------------------------------------------
/**
 * @brief
 * アンオーダードアクセスリソースビュー
 */
class UnorderedAccessView final : public ResourceViewBase {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	ビューを生成する
     * @param	descriptorHeap	ビュー（ディスクリプタ）登録先のヒープ
     * @param	resourceBase	リソース
     */
    void createView(DescriptorHeap& descriptorHeap, ResourceBase* resourceBase) noexcept override;
};

//---------------------------------------------------------------------------------
/**
 * @brief
 * アンオーダードアクセス
 */
template <class T, uint32_t NUM = 1>
class UnorderedAccessObj final : public GpuObj<T, NUM> {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    UnorderedAccessObj() {
        this->resource_.reset(new UnorderedAccessResource());
        this->view_[ViewIndex(ViewType::SRV)] = std::make_unique<ShaderResourceView>();
        this->view_[ViewIndex(ViewType::UAV)] = std::make_unique<UnorderedAccessView>();
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    ~UnorderedAccessObj() = default;
};

}  // namespace dx12::resource
