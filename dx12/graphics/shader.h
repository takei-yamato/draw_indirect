#pragma once

#include "dx12/device.h"

#include "utility/noncopyable.h"

namespace dx12::graphics {
//---------------------------------------------------------------------------------
/**
 * @brief
 * シェーダ
 */
class Shader final : public utility::Noncopyable {
public:
    //---------------------------------------------------------------------------------
    /**
     * @brief	コンストラクタ
     */
    Shader() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	デストラクタ
     */
    ~Shader() = default;

    //---------------------------------------------------------------------------------
    /**
     * @brief	頂点シェーダを取得する
     * @return	頂点シェーダのデータ
     */
    [[nodiscard]] ID3DBlob* vertexShader() const noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	ピクセルシェーダを取得する
     * @return	ピクセルシェーダのデータ
     */
    [[nodiscard]] ID3DBlob* pixelShader() const noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	コンピュートシェーダを取得する
     * @return	コンピュートシェーダのデータ
     */
    [[nodiscard]] ID3DBlob* computeShader() const noexcept;

    //---------------------------------------------------------------------------------
    /**
     * @brief	シェーダを作成する
     * @param	filePath	ファイルパス
     * @return	作成に成功した場合は true
     */
    bool create(std::string_view filePath) noexcept;

private:
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShader_{};
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShader_{};
    Microsoft::WRL::ComPtr<ID3DBlob> computeShader_{};
};
}  // namespace dx12::graphics
