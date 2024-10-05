#include "dx12/resource/unordered_access.h"

namespace {

//---------------------------------------------------------------------------------
/** @def
 * アラインメント
 */
#define ALIGN(size) ((size + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1))
}  // namespace

namespace dx12::resource {


//---------------------------------------------------------------------------------
/**
 * @brief	UAバッファを生成する
 * @param	data		データの先頭アドレス
 * @param	stride		バッファのストライド
 * @param	num			バッファの数
 * @return	作成に成功した場合は true
 */
bool UnorderedAccessResource::create(void** data, uint32_t stride, uint32_t num) noexcept {
    // GPUリソース作成
    D3D12_HEAP_PROPERTIES heapProperty = {};
    heapProperty.Type                  = D3D12_HEAP_TYPE_CUSTOM;
    heapProperty.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    heapProperty.MemoryPoolPreference  = D3D12_MEMORY_POOL_L0;
    heapProperty.CreationNodeMask      = 1;
    heapProperty.VisibleNodeMask       = 1;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment           = 0;
    resourceDesc.Width               = ALIGN((stride * num));
    resourceDesc.Height              = 1;
    resourceDesc.DepthOrArraySize    = 1;
    resourceDesc.MipLevels           = 1;
    resourceDesc.Format              = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count    = 1;
    resourceDesc.SampleDesc.Quality  = 0;
    resourceDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags               = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    auto res = dx12::Device::instance().device()->CreateCommittedResource(
        &heapProperty,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr,
        IID_PPV_ARGS(gpuResource_.GetAddressOf()));
    if (FAILED(res)) {
        ASSERT(false, "UAバッファの作成に失敗");
        return false;
    }

    res = gpuResource_->Map(0, nullptr, data);
    if (FAILED(res)) {
        ASSERT(false, "Map に失敗");
        return false;
    }

    stride_ = stride;
    num_    = num;

    return true;
}

//---------------------------------------------------------------------------------
/**
 * @brief	ビューを生成する
 * @param	descriptorHeap	ビュー（ディスクリプタ）登録先のヒープ
 */
void UnorderedAccessView::createView(DescriptorHeap& descriptorHeap, ResourceBase* resourceBase) noexcept {
    handle_ = descriptorHeap.allocate(1);

    // UAV 作成
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Format                           = DXGI_FORMAT_UNKNOWN;
    uavDesc.Buffer.NumElements               = resourceBase->num();
    uavDesc.Buffer.StructureByteStride       = resourceBase->stride();

    auto handle = handle_.cpuHandle_;
    dx12::Device::instance().device()->CreateUnorderedAccessView(resourceBase->resource(), nullptr, &uavDesc, handle);
}

//---------------------------------------------------------------------------------
/**
 * @brief	コマンドリストに設定する
 * @param	commandList		設定先のコマンドリスト
 * @param	index			バッファのインデックス
 */
void UnorderedAccessView::setToCommandList(dx12::CommandList& commandList, uint32_t index) noexcept {
    // バッファビューの設定
    auto handle = handle_.gpuHandle_;
    commandList.get()->SetComputeRootDescriptorTable(index, handle);
}





}  // namespace dx12::resource
