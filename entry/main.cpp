
#include "window/window.h"

#include "utility/time_counter.h"

#include "dx12/command_queue.h"
#include "dx12/command_list.h"
#include "dx12/device.h"
#include "dx12/swap_chain.h"
#include "dx12/fence.h"

#include "dx12/graphics/pipeline_state_object.h"

#include "dx12/resource/constant_buffer.h"
#include "dx12/resource/mesh.h"
#include "dx12/resource/frame_buffer.h"

#include "input/input.h"

#include <array>
#include <random>
#include <ppl.h>

using namespace dx12;

namespace {
// オブジェクト数
constexpr uint32_t objectNum = 4;

// シーンコンスタントバッファのフォーマット
struct ConstantBufferFormat {
    // ビュープロジェクション
    DirectX::XMMATRIX viewProj{};

    // メッシュ用の情報
    DirectX::XMMATRIX world[objectNum]{};
    DirectX::XMFLOAT4 color[objectNum]{};

    // 描画対象のインデックス
    uint32_t index[objectNum]{};
};

// 頂点フォーマット
struct Vertex {
    DirectX::XMFLOAT3 pos{};
    DirectX::XMFLOAT2 uv{};
};

// 頂点データ
Vertex vertexData[] = {
    { {-0.5f, 0.5f, 0.0f}, {0, 0}},
    {  {0.5f, 0.5f, 0.0f}, {1, 1}},
    {{-0.5f, -0.5f, 0.0f}, {0, 1}},
    { {0.5f, -0.5f, 0.0f}, {1, 0}}
};

// インデックスデータ
uint16_t indexData[] = {0, 1, 2, 2, 1, 3};

// カメラ
DirectX::XMFLOAT3 eye(0.0f, 0.0f, -20.0f);
DirectX::XMFLOAT3 dir(0.0f, 0.0f, 1.0f);
DirectX::XMFLOAT3 up(0.0f, 1.0f, 0.0f);
DirectX::XMMATRIX view{};
DirectX::XMMATRIX proj{};

// アスペクト比
float aspect = static_cast<float>(window::width()) / static_cast<float>(window::Height());

// 各オブジェクトのワールド行列
DirectX::XMMATRIX world[objectNum]{};

// 各オブジェクトのカラー
DirectX::XMFLOAT4 color[objectNum]{};

// フレームバッファ(二つ分)
resource::FrameBuffer frameBuffer(2);

// メッシュ
resource::Mesh mesh{};

// コンスタントバッファ
resource::ConstantBuffer<ConstantBufferFormat, 1> sceneConstantBuffer{};

// パイプラインステートオブジェクト
graphics::PipelineStateObject pso{};

// コマンドキュー
CommandQueue commandQueue{};

// フェンス
Fence fence{};

// コマンドリスト
CommandList commandListBegin{};
CommandList commandListDraw{};
CommandList commandListEnd{};
}  // namespace

namespace {
//---------------------------------------------------------------------------------
/**
 * @brief	アプリケーションの更新処理を行う
 */
bool appUpdate() noexcept {
    if (window::Window::instance().isEnd()) {
        return false;
    }

    {
        TIME_CHECK_SCORP("更新時間");

        // 描画対象を選ぶ
        auto drawObjNum = 0;
        {
            // 特定のキーを押下すると対応する対象が表示されなくなる
            bool disable[] = {
                input::Input::instance().getKey('1'),
                input::Input::instance().getKey('2'),
                input::Input::instance().getKey('3'),
                input::Input::instance().getKey('4'),
            };

            for (auto i = 0; i < objectNum; ++i) {
                if (disable[i]) {
                    continue;
                }
                sceneConstantBuffer[0].index[drawObjNum] = i;
                drawObjNum++;
            }
        }

        // 描画開始
        {
            commandListBegin.reset();
            frameBuffer.startRendering(commandListBegin);
            commandListBegin.get()->Close();
        }

        // 各メッシュ描画
        {
            commandListDraw.reset();

            frameBuffer.setToRenderTarget(commandListDraw);
            pso.setToCommandList(commandListDraw);
            mesh.setToCommandList(commandListDraw);
            sceneConstantBuffer.setToCommandList(commandListDraw, 0);

            // インスタンス描画
            commandListDraw.get()->DrawIndexedInstanced(6, drawObjNum, 0, 0, 0);

            commandListDraw.get()->Close();
        }

        // 描画終了
        {
            commandListEnd.reset();
            frameBuffer.finishRendering(commandListEnd);
            commandListEnd.get()->Close();
        }

        // コマンドリスト実行
        {
            std::array<ID3D12CommandList*, 3> lists;

            // 描画開始コマンドリスト
            lists[0] = commandListBegin.get();
            // 各メッシュ描画コマンドリスト
            lists[1] = commandListDraw.get();
            // 描画終了コマンドリスト
            lists[2] = commandListEnd.get();

            // コマンドリスト実行
            commandQueue.get()->ExecuteCommandLists(lists.size(), static_cast<ID3D12CommandList**>(lists.data()));
            SwapChain::instance().present();

            // フレームバッファのインデックスを更新する
            frameBuffer.updateBufferIndex(SwapChain::instance().currentBufferIndex());
        }
    }

    // 時間表示
    TIME_PRINT("");

    // フェンス設定
    {
        fence.get()->Signal(0);
        commandQueue.get()->Signal(fence.get(), 1);
    }

    // GPU処理が全て終了するまでCPUを待たせる
    {
        auto event = CreateEvent(nullptr, false, false, "WAIT_GPU");
        fence.get()->SetEventOnCompletion(1, event);
        WaitForSingleObject(event, INFINITE);
        CloseHandle(event);
    }

    return true;
}

}  // namespace

//---------------------------------------------------------------------------------
/**
 * @brief	エントリー関数
 */
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT) {
    std::random_device                    rd;
    std::default_random_engine            re(rd());
    std::uniform_real_distribution<float> distr(-5.0f, 5.0f);

    {
        // メモリリークチェック
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        {
            TRACE("アプリケーション開始");

            // ウィンドウ生成
            window::Window::instance().create(hInstance);
            window::Window::instance().wait();

            // dx12 デバイスを作成する
            Device::instance().create();

            // コマンドキューを作成する
            commandQueue.create();

            // スワップチェインを作成する
            SwapChain::instance().create(commandQueue, frameBuffer);

            // メッシュを作成する
            mesh.createVertexBuffer(vertexData);
            mesh.createIndexBuffer(indexData);


            // ビュー行列
            view = DirectX::XMMatrixLookToLH(XMLoadFloat3(&eye), XMLoadFloat3(&dir), XMLoadFloat3(&up));
            // プロジェクション行列
            proj = DirectX::XMMatrixPerspectiveFovLH(3.14159f / 4.f, aspect, 0.1f, 1000.0f);
            // ワールド行列
            world[0] = DirectX::XMMatrixTranslation(-3.0f, 0, 0);
            world[1] = DirectX::XMMatrixTranslation(-1.0f, 0, 0);
            world[2] = DirectX::XMMatrixTranslation(1.0f, 0, 0);
            world[3] = DirectX::XMMatrixTranslation(3.0f, 0, 0);
            // カラー
            color[0] = DirectX::XMFLOAT4(1.0f, 0, 0, 1.0f);
            color[1] = DirectX::XMFLOAT4(0, 1.0f, 0, 1.0f);
            color[2] = DirectX::XMFLOAT4(0, 0, 1.0f, 1.0f);
            color[3] = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

            // コンスタントバッファを作成する
            sceneConstantBuffer.createBuffer();
            // シーンコンスタントバッファの内容を設定する
            sceneConstantBuffer[0].viewProj = DirectX::XMMatrixTranspose(view * proj);
			// シーンに登場するオブジェクト毎の内容を設定する
            for (auto i = 0; i < objectNum; ++i) {
                sceneConstantBuffer[0].world[i] = DirectX::XMMatrixTranspose(world[i]);
                sceneConstantBuffer[0].color[i] = color[i];
                sceneConstantBuffer[0].index[i] = i;
            }

            // フェンス（CPUとGPUの同期オブジェクト）を作成する
            fence.create();

            // 各コマンドリストを作成する
            commandListBegin.create();
            commandListDraw.create();
            commandListEnd.create();

            // パイプラインステートオブジェクトを作成する
            pso.create();

            // アプリケーションループ
            while (appUpdate()) {
            }
        }
    }

    return 0;
}
