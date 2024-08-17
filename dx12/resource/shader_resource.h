#pragma once

#include "dx12/device.h"
#include "dx12/command_list.h"
#include "dx12/gpu_resource.h"

#include "utility/noncopyable.h"

namespace dx12::resource {

//---------------------------------------------------------------------------------
/**
 * @brief
 * シェーダリソース
 */
class ShaderResource final : public ResourceBase {
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
 * シェーダリソースビュー
 */
class ShaderResourceView final : public ResourceViewBase {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	ビューを生成する
     * @param	descriptorHeap	ビュー（ディスクリプタ）登録先のヒープ
     * @param	resourceBase	リソース
     */
    void createView(DescriptorHeap& descriptorHeap, ResourceBase* resourceBase) noexcept override;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストに設定する
     * @param	commandList		設定先のコマンドリスト
     * @param	index			バッファのインデックス
     */
    void setToCommandList(dx12::CommandList& commandList, uint32_t index) noexcept override;
};

//---------------------------------------------------------------------------------
/**
 * @brief
 * シェーダリソース
 */
template <class T, uint32_t NUM = 1>
class ShaderResourceObj final : public GpuObj<T, NUM> {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    ShaderResourceObj() {
        this->resource_.reset(new ShaderResource());
        this->view_.reset(new ShaderResourceView());
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    ~ShaderResourceObj() = default;
};

}  // namespace dx12::resource
