#pragma once

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
    ResourceBase()          = default;
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
     * @brief	バッファのストライド
     * @return	ストライドサイズ
     */
    uint32_t stride() const noexcept {
        return stride_;
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	バッファ要素数
     * @return	要素数
     */
    uint32_t num() const noexcept {
        return num_;
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	オフセットを取得する
     * @param	index			バッファのインデックス
     * @return	インデックスに対応するオフセット
     */
    uint32_t offset(uint32_t index) const noexcept {
        return index * stride_;
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	リソースを取得する
     * @return	リソース
     */
    ID3D12Resource* resource() const noexcept {
        return gpuResource_.Get();
    }

protected:
    Microsoft::WRL::ComPtr<ID3D12Resource> gpuResource_{};  ///< リソース
    uint32_t                               stride_{};       ///< バッファのストライド
    uint32_t                               num_{};          ///< バッファ数
};

//---------------------------------------------------------------------------------
/**
 * @brief
 * GPU リソースビュー
 */
class ResourceViewBase : public utility::Noncopyable {
public:
    ResourceViewBase()          = default;
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
     * @param	index			バッファのインデックス
     */
    virtual void setToCommandList(dx12::CommandList& commandList, uint32_t index) noexcept = 0;

protected:
    DescriptorHeap::Handle handle_{};  ///< ディスクリプタハンドル
};


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
    GpuObj() = default;

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
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	ビューを生成する
     * @param	descriptorHeap	ビュー（ディスクリプタ）登録先のヒープ
     */
    void createView(DescriptorHeap& descriptorHeap) noexcept {
        view_->createView(descriptorHeap, resource_.get());
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストに設定する
     * @param	commandList		設定先のコマンドリスト
     * @param	index			バッファのインデックス
     */
    void setToCommandList(CommandList& commandList, uint32_t index) noexcept {
        view_->setToCommandList(commandList, index);
    }

    //---------------------------------------------------------------------------------
    /**
     * @brief	バッファのデータを取得する
     * @param	index			データインデックス
     * @return	データの参照
     */
    T& operator[](uint32_t index) const noexcept {
        auto* address = reinterpret_cast<char*>(data_) + resource_->offset(index);
        return *(reinterpret_cast<T*>(address));
    }

protected:
    std::unique_ptr<ResourceBase>     resource_{};  ///< リソース
    std::unique_ptr<ResourceViewBase> view_{};      ///< ビュー
    T*                                data_{};      ///< CPUで内容を変更する際のアクセス先アドレス
};

}  // namespace dx12
