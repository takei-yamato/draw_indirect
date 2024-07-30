#pragma once

#include "dx12/device.h"
#include "dx12/command_list.h"

#include "dx12/descriptor_heap.h"

#include "utility/noncopyable.h"

namespace dx12::resource {

//---------------------------------------------------------------------------------
/**
 * @brief
 * コンスタントバッファリソース
 */
class Constant final : public utility::Noncopyable {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    Constant() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    ~Constant() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コンスタントバッファを生成する
     * @param	data		データの先頭アドレス
     * @param	stride		コンスタントバッファのストライド
     * @param	num			コンスタントバッファの数
     * @return	作成に成功した場合は true
     */
    bool create(void** data, uint32_t stride, uint32_t num) noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	ビューを生成する
     * @param	descriptorHeap	ビュー（ディスクリプタ）登録先のヒープ
     */
    void createView(DescriptorHeap& descriptorHeap) noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストに設定する
     * @param	commandList		設定先のコマンドリスト
     * @param	index			コンスタントバッファのインデックス
     */
    void setToCommandList(CommandList& commandList, uint32_t index) noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	オフセットを取得する
     * @param	index			コンスタントバッファのインデックス
     * @return	インデックスに対応するオフセット
     */
    uint64_t offset(uint32_t index) const noexcept;

private:
    DescriptorHeap::Handle                       handle_{};
    Microsoft::WRL::ComPtr<ID3D12Resource>       gpuResource_{};  ///< リソース
    uint32_t                                     stride_{};       ///< バッファのストライド
    uint32_t                                     num_{};          ///< バッファ数
};

}  // namespace dx12::resource
