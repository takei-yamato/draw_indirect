#include "dx12/descriptor_heap.h"
#include <format>

namespace dx12 {

//---------------------------------------------------------------------------------
/**
 * @brief	ディスクリプタヒープを生成する
 * @param	type			ヒープが管理する対象の種類
 * @param	capacity		最大管理数
 * @return	作成に成功した場合は true
 */

bool DescriptorHeap::create(Type type, uint32_t capacity) noexcept {
    auto flag = type == Type::CBV_SRV_UAV ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                                          : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    // ディスクリプタヒープ作成
    desc_                = {};
    desc_.Type           = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(type);
    desc_.NumDescriptors = capacity;
    desc_.Flags          = flag;

    auto res = dx12::Device::instance().device()->CreateDescriptorHeap(
        &desc_,
        IID_PPV_ARGS(heap_.GetAddressOf()));

    if (FAILED(res)) {
        ASSERT(false, "ディスクリプタヒープの作成に失敗");
        return false;
    }
    heap_->SetName(std::format(L"DescriptorHeap type:{} capacity:{} ", static_cast<uint32_t>(type), capacity).data());

    capacity_ = capacity;
    return true;
}

//---------------------------------------------------------------------------------
/**
 * @brief	ヒープから指定数を確保する
 * @param	num			確保数
 * @return	CPU と GPU のディスクリプタハンドル
 */
DescriptorHeap::Handle DescriptorHeap::allocate(uint32_t num) noexcept {
    const auto temp = currentIndex_;
    currentIndex_ += num;

    return handleFromIndex(temp);
}

//---------------------------------------------------------------------------------
/**
 * @brief	インデックスを指定してハンドルを取得する
 * @param	index		インデックス
 * @return	CPU と GPU のディスクリプタハンドル
 */
DescriptorHeap::Handle DescriptorHeap::handleFromIndex(uint32_t index) noexcept {
    const auto size = dx12::Device::instance().device()->GetDescriptorHandleIncrementSize(desc_.Type);

    auto cpuHandle = heap_->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += (index * size);

    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};
    if (desc_.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
        gpuHandle = heap_->GetGPUDescriptorHandleForHeapStart();
        gpuHandle.ptr += (index * size);
    }

    return {index, cpuHandle, gpuHandle, size};
}

//---------------------------------------------------------------------------------
/**
 * @brief	ヒープをコマンドリストに設定する
 * @param	commandList				設定先のコマンドリスト
 */
void DescriptorHeap::setToCommandList(dx12::CommandList& commandList) noexcept {
    // ヒープの設定
    ID3D12DescriptorHeap* p[] = {heap_.Get()};
    commandList.get()->SetDescriptorHeaps(_countof(p), p);
}

}  // namespace dx12
