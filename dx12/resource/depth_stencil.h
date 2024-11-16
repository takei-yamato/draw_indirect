#pragma once

#include "dx12/command_list.h"
#include "dx12/gpu_resource.h"

#include "utility/noncopyable.h"

namespace dx12::resource {

//---------------------------------------------------------------------------------
/**
 * @brief
 * デプスステンシルリソース
 */
class DepthStencilResource final : public ResourceBase {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    DepthStencilResource() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    ~DepthStencilResource() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デプスステンシルを作成する
     * @return	作成に成功した場合は true
     */
    bool create() noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	リソースを作成するを生成する
     * @param	data		データの先頭アドレス
     * @param	stride		バッファのストライド
     * @param	num			バッファの数
     * @return	作成に成功した場合は true
     */
    bool create(void** data, uint32_t stride, uint32_t num) noexcept override { return false; }
};

//---------------------------------------------------------------------------------
/**
 * @brief
 * デプスステンシル
 */
class DepthStencil final : public utility::Noncopyable {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    DepthStencil() { resource_ = std::make_unique<DepthStencilResource>(); }

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    ~DepthStencil() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デプスステンシルを作成する
     * @return	作成に成功した場合は true
     */
    [[nodiscard]] bool create() noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	ビューを生成する
     * @param	descriptorHeap	ビュー（ディスクリプタ）登録先のヒープ
     */
    void createView(DescriptorHeap& descriptorHeap) noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デプスステンシルビューを取得する
     * @return	デプスステンシルビュー（ハンドル）
     */
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE view() noexcept;

private:
    DescriptorHeap                        heap_{};      ///< ディスクリプタヒープ
    DescriptorHandle                      handle_{};    ///< ディスクリプタハンドル
    std::unique_ptr<DepthStencilResource> resource_{};  ///< リソース
};
}  // namespace dx12::resource
