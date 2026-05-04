#pragma once

#include "dx12/device.h"

#include "utility/noncopyable.h"

namespace dx12::graphics {
//---------------------------------------------------------------------------------
/**
 * @brief
 * ルートシグネチャ
 */
class RootSignature : public utility::Noncopyable {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    RootSignature() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    virtual ~RootSignature() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	ルートシグネチャを作成する
     * @return	作成に成功した場合は true
     */
    virtual bool create() noexcept = 0;

public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	ルートシグネチャを取得する
     * @return　ルートシグネチャ
     */
	ID3D12RootSignature* get() const noexcept {
        return rootSignature_.Get();
	}

protected:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_{};  ///< ルートシグネチャ
};
}  // namespace dx12::graphics
