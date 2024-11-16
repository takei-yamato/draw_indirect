#include "dx12/resource/depth_stencil.h"

#include "window/window.h"

namespace dx12::resource {

//---------------------------------------------------------------------------------
/**
 * @brief	デプスステンシルを作成する
 * @return	作成に成功した場合は true
 */
bool DepthStencilResource::create() noexcept {
    D3D12_CLEAR_VALUE dsvClearValue{};
    dsvClearValue.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvClearValue.DepthStencil.Depth   = 1.0f;
    dsvClearValue.DepthStencil.Stencil = 0;

    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment          = 0;
    resourceDesc.Width              = window::width();
    resourceDesc.Height             = window::height();
    resourceDesc.DepthOrArraySize   = 1;
    resourceDesc.MipLevels          = 1;
    resourceDesc.Format             = dsvClearValue.Format;
    resourceDesc.SampleDesc.Count   = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

    D3D12_HEAP_PROPERTIES heapProperty{};
    heapProperty.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    heapProperty.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperty.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperty.CreationNodeMask     = 1;
    heapProperty.VisibleNodeMask      = 1;

    auto res = dx12::Device::instance().device()->CreateCommittedResource(&heapProperty, D3D12_HEAP_FLAG_NONE,
                                                                          &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                                          &dsvClearValue, IID_PPV_ARGS(gpuResource_.GetAddressOf()));
    if (FAILED(res)) {
        ASSERT(false, "デプスステンシルバッファの作成に失敗");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------
/**
 * @brief	デプスステンシルを作成する
 * @return	作成に成功した場合は true
 */
bool DepthStencil::create() noexcept {
    // リソースを作成する
    resource_->create();

    // ビューを格納するディスクリプタヒープを作成する
    heap_.create(dx12::DescriptorHeap::Type::DSV, 1);

    createView(heap_);

    return true;
}

//---------------------------------------------------------------------------------
/**
 * @brief	ビューを生成する
 * @param	descriptorHeap	ビュー（ディスクリプタ）登録先のヒープ
 */
void DepthStencil::createView(DescriptorHeap& descriptorHeap) noexcept {
    // ハンドルを取得する
    handle_ = descriptorHeap.allocate(1);

    // ディスクリプタを作成
    dx12::Device::instance().device()->CreateDepthStencilView(resource_->resource(), nullptr, handle_.cpuHandle_);
}

//---------------------------------------------------------------------------------
/**
 * @brief	デプスステンシルビューを取得する
 * @return	デプスステンシルビュー（ハンドル）
 */
D3D12_CPU_DESCRIPTOR_HANDLE DepthStencil::view() noexcept {
    return handle_.cpuHandle_;
}

}  // namespace dx12::resource
