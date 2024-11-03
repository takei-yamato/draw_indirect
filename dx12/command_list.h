#pragma once

#include "dx12/device.h"
#include "dx12/descriptor_handle.h"
#include "utility/noncopyable.h"

namespace dx12 {

//---------------------------------------------------------------------------------
/**
 * @brief	コマンド種類
 */
enum class CommandType {
    Graphics,
    Compute,
    Copy,
};

//---------------------------------------------------------------------------------
/**
 * @brief
 * コマンドリスト
 */
class CommandList final : public utility::Noncopyable {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    CommandList() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    ~CommandList();

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストを作成する
	 * @param	type コマンドリストが扱う種類
     * @return	作成に成功した場合は true
     */
    bool create(CommandType type) noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストをリセットする
     */
    void reset() noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	ルートパラメータを設定する
	 * @param	handle ディスクリプタハンドル
     * @return
     */
    void setRootParameters(uint32_t index, const DescriptorHandle& handle) noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドリストを取得する
     */
    [[nodiscard]] ID3D12GraphicsCommandList* get() const noexcept;

private:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    commandAllocator_{};  ///< コマンドアロケータ
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_{};       ///< コマンドリスト
    CommandType                                       type_{};              ///< コマンドリストが扱う種類
};
}  // namespace dx12
