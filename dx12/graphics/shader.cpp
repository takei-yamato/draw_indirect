#include "dx12/graphics/shader.h"
#include "dx12/device.h"

#include <D3Dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

namespace dx12::graphics {

//---------------------------------------------------------------------------------
/**
 * @brief	頂点シェーダを取得する
 * @return	頂点シェーダのデータ
 */
ID3DBlob* Shader::vertexShader() const noexcept {
    ASSERT(vertexShader_, "シェーダーがありません");
    return vertexShader_.Get();
}

//---------------------------------------------------------------------------------
/**
 * @brief	ピクセルシェーダを取得する
 * @return	ピクセルシェーダのデータ
 */
ID3DBlob* Shader::pixelShader() const noexcept {
    ASSERT(pixelShader_, "シェーダーがありません");
    return pixelShader_.Get();
}

//---------------------------------------------------------------------------------
/**
 * @brief	コンピュートシェーダを取得する
 * @return	コンピュートシェーダのデータ
 */
ID3DBlob* Shader::computeShader() const noexcept {
    ASSERT(computeShader_, "シェーダーがありません");
    return computeShader_.Get();
}

//---------------------------------------------------------------------------------
/**
 * @brief	シェーダを作成する
 * @param	filePath	ファイルパス
 * @return	作成に成功した場合は true
 */
bool Shader::create(std::string_view filePath) noexcept {
    // シェーダ作成
    Microsoft::WRL::ComPtr<ID3DBlob> error;

    auto r    = false;
    auto temp = std::wstring(filePath.begin(), filePath.end());

    auto res = D3DCompileFromFile(temp.data(), nullptr, nullptr, "vs", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, vertexShader_.GetAddressOf(), error.GetAddressOf());
    if (FAILED(res)) {
        char* p = static_cast<char*>(error->GetBufferPointer());
        TRACE(p);
    } else {
        r = true;
    }

    res = D3DCompileFromFile(temp.data(), nullptr, nullptr, "ps", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, pixelShader_.GetAddressOf(), error.GetAddressOf());
    if (FAILED(res)) {
        char* p = static_cast<char*>(error->GetBufferPointer());
        TRACE(p);
    } else {
        r = true;
    }

    res = D3DCompileFromFile(temp.data(), nullptr, nullptr, "cs", "cs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, computeShader_.GetAddressOf(), error.GetAddressOf());
    if (FAILED(res)) {
        char* p = static_cast<char*>(error->GetBufferPointer());
        TRACE(p);
    } else {
        r = true;
    }

	ASSERT(r, "シェーダの作成に失敗");

    return r;
}

}  // namespace dx12::graphics
