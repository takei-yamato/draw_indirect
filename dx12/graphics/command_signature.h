#pragma once

#include "dx12/device.h"
#include "dx12/graphics/root_signature.h"

#include "utility/noncopyable.h"

namespace dx12::graphics {
//---------------------------------------------------------------------------------
/**
 * @brief コマンドシグネチャ
 */
class CommandSignature : public utility::Noncopyable {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    CommandSignature() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    virtual ~CommandSignature() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドシグネチャを作成する
	 * @param	rootSignature	ルートシグネチャ
	 * @param	commandSize		コマンドのサイズ（バイト単位）
     * @return	作成に成功した場合は true
     */
    bool create(const RootSignature* rootSignature, uint32_t commandSize) noexcept;

public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コマンドシグネチャを取得する
     * @return　コマンドシグネチャ
     */
    ID3D12CommandSignature* get() const noexcept {
        return commandSignature_.Get();
    }

protected:
    Microsoft::WRL::ComPtr<ID3D12CommandSignature> commandSignature_{};  ///< コマンドシグネチャ
};
}  // namespace dx12::graphics
