#pragma once

#include "dx12/device.h"

#include "utility/noncopyable.h"

namespace dx12 {
//---------------------------------------------------------------------------------
/**
 * @brief
 * コマンドキュー
 */
class CommandQueue final : utility::Noncopyable {
public:

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドキューの対象種類
     */
    enum class Type {
        Graphics,
        Compute,
    };

public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    CommandQueue() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    ~CommandQueue() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドキューを生成する
     * @param	type		対象の種類
     * @return	正しく生成できた場合は true
     */
    bool create(Type type) noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドキューを取得する
     */
    [[nodiscard]] ID3D12CommandQueue* get() const noexcept;

private:
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;  ///< コマンドキュー
};
}  // namespace dx12
