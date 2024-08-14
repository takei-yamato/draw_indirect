#pragma once

#include "dx12/device.h"
#include "dx12/command_list.h"
#include "dx12/descriptor_heap.h"

#include "dx12/resource/shader_resource.h"
#include "dx12/resource/unordered_access.h"
#include "dx12/resource/constant.h"

#include "utility/noncopyable.h"

namespace dx12 {

//---------------------------------------------------------------------------------
/**
 * @brief
 * GPUリソース
 */
template <class R, class T, uint32_t Num>
class Resource final : utility::Noncopyable {
private:
    using type = T;
    using res  = R;

public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    Resource() {
        resource_.reset(new res());
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    ~Resource() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	バッファを作成する
     */
    void create() noexcept {
        resource_->create(reinterpret_cast<void**>(&data_), sizeof(type), Num);
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	ビューを生成する
     * @param	descriptorHeap	ビュー（ディスクリプタ）登録先のヒープ
     */
    void createView(DescriptorHeap& descriptorHeap) noexcept {
        resource_->createView(descriptorHeap);
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストに設定する
     * @param	commandList		設定先のコマンドリスト
     * @param	index			バッファのインデックス
     */
    void setToCommandList(CommandList& commandList, uint32_t index) noexcept {
        resource_->setToCommandList(commandList, index);
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	バッファのデータを取得する
     * @param	index			データインデックス
     * @return	データの参照
     */
    type& operator[](uint32_t index) const noexcept {
        auto* address = reinterpret_cast<char*>(data_) + resource_->offset(index);
        return *(reinterpret_cast<type*>(address));
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	バッファのストライド
     * @return	ストライドサイズ
     */
    uint32_t stride() const noexcept {
        return sizeof(type);
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	バッファ要素数
     * @return	要素数
     */
    uint32_t num() const noexcept {
        return Num;
    }

private:
    std::unique_ptr<res> resource_{};
    type*                data_{};  ///< CPUで内容を変更する際のアクセス先アドレス
};
}  // namespace dx12
