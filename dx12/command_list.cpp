#include "dx12/command_list.h"

namespace dx12 {

//---------------------------------------------------------------------------------
/**
 * @brief	デストラクタ
 */
CommandList::~CommandList() {
    reset();
}

//---------------------------------------------------------------------------------
/**
 * @brief	コマンドリストを作成する
 * @param	type コマンドリストが扱う種類
 * @return	作成に成功した場合は true
 */
bool CommandList::create(CommandType type) noexcept {
    type_  = type;
    auto t = type == CommandType::Compute ? D3D12_COMMAND_LIST_TYPE_COMPUTE : D3D12_COMMAND_LIST_TYPE_DIRECT;

    // アロケータ作成
    auto res = Device::instance().device()->CreateCommandAllocator(
        t,
        IID_PPV_ARGS(commandAllocator_.GetAddressOf()));
    if (FAILED(res)) {
        ASSERT(false, "コマンドアロケータ作成に失敗");
        return false;
    }

    // コマンドリスト作成
    res = Device::instance().device()->CreateCommandList(
        0,
        t,
        commandAllocator_.Get(), nullptr, IID_PPV_ARGS(commandList_.GetAddressOf()));
    if (FAILED(res)) {
        ASSERT(false, "コマンドリスト作成に失敗");
        return false;
    }

    commandList_->SetName(L"CommandAlloc");

    commandList_->Close();

    return true;
}

//---------------------------------------------------------------------------------
/**
 * @brief	コマンドリストをリセットする
 */
void CommandList::reset() noexcept {
    // コマンドアロケータをリセット
    commandAllocator_->Reset();

    // コマンドリセット
    commandList_->Reset(commandAllocator_.Get(), nullptr);
}

//---------------------------------------------------------------------------------
/**
 * @brief	ルートパラメータを設定する
 * @param	index	ルートパラメータのインデックス
 * @param	handle	ディスクリプタハンドル
 * @return
 */
void CommandList::setRootParameters(uint32_t index, const DescriptorHandle& handle) noexcept {
    if (type_ == CommandType::Compute) {
        get()->SetComputeRootDescriptorTable(index, handle.gpuHandle_);
    } else {
        get()->SetGraphicsRootDescriptorTable(index, handle.gpuHandle_);
    }
}

//---------------------------------------------------------------------------------
/**
 * @brief	コマンドリストを取得する
 */
ID3D12GraphicsCommandList* CommandList::get() const noexcept {
    return commandList_.Get();
}

}  // namespace dx12
