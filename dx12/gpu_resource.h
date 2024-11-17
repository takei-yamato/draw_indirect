#pragma once

#include <utility>

#include "dx12/device.h"
#include "dx12/command_list.h"
#include "dx12/descriptor_heap.h"

#include "utility/noncopyable.h"

namespace dx12 {

//---------------------------------------------------------------------------------
/**
 * @brief
 * GPU リソース
 */
class ResourceBase : public utility::Noncopyable {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    ResourceBase() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    virtual ~ResourceBase() = default;

public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	リソースを作成するを生成する
     * @param	data		データの先頭アドレス
     * @param	stride		バッファのストライド
     * @param	num			バッファの数
     * @return	作成に成功した場合は true
     */
    virtual bool create(void** data, uint32_t stride, uint32_t num) noexcept = 0;

    //---------------------------------------------------------------------------------
    /**
     * @brief	マップする
     * @param	data		データの先頭アドレス
     */
    void map(void** data) noexcept {
        if (mapping_) {
            return;
        }
        auto res = gpuResource_->Map(0, nullptr, data);
        if (FAILED(res)) {
            ASSERT(false, "Map に失敗");
            return;
        }

        mapping_ = true;
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	マップを解除する
     */
    void unmap() noexcept {
        if (!mapping_) {
            return;
        }
        gpuResource_.Get()->Unmap(0, nullptr);
        mapping_ = false;
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	バッファのストライド
     * @return	ストライドサイズ
     */
    uint32_t stride() const noexcept { return stride_; }

    //---------------------------------------------------------------------------------
    /**
     * @brief	バッファ要素数
     * @return	要素数
     */
    uint32_t num() const noexcept { return num_; }

    //---------------------------------------------------------------------------------
    /**
     * @brief	オフセットを取得する
     * @param	index			バッファのインデックス
     * @return	インデックスに対応するオフセット
     */
    uint32_t offset(uint32_t index) const noexcept { return index * stride_; }

    //---------------------------------------------------------------------------------
    /**
     * @brief	リソースを取得する
     * @return	リソース
     */
    ID3D12Resource* resource() const noexcept { return gpuResource_.Get(); }

    //---------------------------------------------------------------------------------
    /**
     * @brief	マップ中か
     * @return	リソース
     */
    bool mapping() const noexcept { return mapping_; }

protected:
    Microsoft::WRL::ComPtr<ID3D12Resource> gpuResource_{};  ///< リソース
    uint32_t                               stride_{};       ///< バッファのストライド
    uint32_t                               num_{};          ///< バッファ数
    bool                                   mapping_{};      ///< マップ中か
};

//---------------------------------------------------------------------------------
/**
 * @brief
 * GPU リソースビュー
 */
class ResourceViewBase : public utility::Noncopyable {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    ResourceViewBase() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    virtual ~ResourceViewBase() = default;

public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	ビューを生成する
     * @param	descriptorHeap	ビュー（ディスクリプタ）登録先のヒープ
     * @param	resourceBase	リソース
     */
    virtual void createView(DescriptorHeap& descriptorHeap, ResourceBase* resourceBase) noexcept = 0;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストに設定する
     * @param	commandList		設定先のコマンドリスト
     * @param	rootParamIndex	ルートパラメータのインデックス
     */
    void setToCommandList(dx12::CommandList& commandList, uint32_t rootParamIndex) noexcept {
        commandList.setRootParameters(rootParamIndex, handle_);
    }

protected:
    DescriptorHandle handle_{};  ///< ディスクリプタハンドル
};

//---------------------------------------------------------------------------------
/**
 * @brief
 * リソースビュータイプ
 */
enum class ViewType : uint8_t {
	CBV,
    SRV,
    UAV,

    Count,
};
template <class T>
constexpr std::underlying_type_t<T> ViewIndex(T type) {
    return static_cast<std::underlying_type_t<T>>(type);
}

//---------------------------------------------------------------------------------
/**
 * @brief
 * GPU オブジェクト
 */
template <class T, uint32_t NUM>
class GpuObj : public utility::Noncopyable {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
	GpuObj() {
        view_.resize(ViewIndex(ViewType::Count));
	}

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    virtual ~GpuObj() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	リソースを生成する
     */
    void create() noexcept {
        resource_->create(reinterpret_cast<void**>(&data_), sizeof(T), NUM);
        map();
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	ビューを生成する
     * @param	viewType		ビュー
     * @param	descriptorHeap	ビュー（ディスクリプタ）登録先のヒープ
     */
    void createView(ViewType viewType, DescriptorHeap& descriptorHeap) noexcept {
        ASSERT(view_[ViewIndex(viewType)] != nullptr, "ビューがありません");
        view_[ViewIndex(viewType)]->createView(descriptorHeap, resource_.get());
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストに設定する
     * @param	commandList		設定先のコマンドリスト
     * @param	viewType		ビュー
     * @param	rootParamIndex	ルートパラメータのインデックス
     */
    void setToCommandList(CommandList& commandList, ViewType viewType, uint32_t rootParamIndex) noexcept {
        ASSERT(view_[ViewIndex(viewType)] != nullptr, "ビューがありません");

		unmap();
        view_[ViewIndex(viewType)]->setToCommandList(commandList, rootParamIndex);
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	リソースバリア
     * @param	commandList		設定先のコマンドリスト
     * @param	type			バリアタイプ
     * @param	before			遷移前の状態
     * @param	after			遷移後の状態
     */
    void resourceBarrier(CommandList& commandList, D3D12_RESOURCE_BARRIER_TYPE type, D3D12_RESOURCE_STATES before = D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATES after = D3D12_RESOURCE_STATE_COMMON) noexcept {
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type                 = type;
        barrier.Transition.pResource = resource_->resource();

        if (type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
            barrier.Transition.StateBefore = before;
            barrier.Transition.StateAfter  = after;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        }
        commandList.get()->ResourceBarrier(1, &barrier);
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	マップする
     */
    void map() noexcept {
        resource_->map(reinterpret_cast<void**>(&data_));
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	マップを解除する
     */
    void unmap() noexcept {
        resource_->unmap();
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	バッファのデータを取得する
     * @param	index			データインデックス
     * @return	データの参照
     */
    T& operator[](uint32_t index) const noexcept {
        ASSERT(resource_->mapping(), "CPU からアクセスできません");
        auto* address = reinterpret_cast<char*>(data_) + resource_->offset(index);
        return *(reinterpret_cast<T*>(address));
    }

protected:
    std::unique_ptr<ResourceBase>                  resource_{};  ///< リソース
    std::vector<std::unique_ptr<ResourceViewBase>> view_{};      ///< ビュー
    T*                                             data_{};      ///< CPUで内容を変更する際のアクセス先アドレス
};

}  // namespace dx12
