
#include "window/window.h"

#include "utility/time_counter.h"

#include "dx12/command_queue.h"
#include "dx12/command_list.h"
#include "dx12/descriptor_heap.h"
#include "dx12/device.h"
#include "dx12/swap_chain.h"
#include "dx12/fence.h"
#include "dx12/resource/shader_resource.h"
#include "dx12/resource/constant.h"

#include "dx12/graphics/graphics_pipeline_state_object.h"

#include "dx12/resource/mesh.h"
#include "dx12/resource/frame_buffer.h"

#include "camera/frustum.h"

#include "input/input.h"

#include <array>
#include <random>
#include <ppl.h>
#include <format>

using namespace dx12;

namespace {

// フレームバッファ数
constexpr uint32_t frameBufferNum = 2;

// インスタンス数（同一の物を一度に描画する数）
constexpr uint32_t instanceNum = 100000;

// シーンコンスタントバッファのフォーマット
struct ConstantBufferFormat {
    // ビュープロジェクション
    DirectX::XMMATRIX viewProj{};
};

// インスタンス情報バッファのフォーマット
struct InstanceBufferFormat {
    // オブジェクト用の情報
    DirectX::XMMATRIX world{};
    DirectX::XMFLOAT4 color{};
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

// フレームバッファ
resource::FrameBuffer frameBuffer(frameBufferNum);

// ディスクリプタヒープ
DescriptorHeap descriptorHeap{};
// メッシュ
resource::Mesh mesh{};
// シーンコンスタントバッファ
resource::ConstantBufferObj<ConstantBufferFormat> sceneConstantBuffer{};
// インスタンス情報バッファ
resource::ShaderResourceObj<InstanceBufferFormat, instanceNum> instanceBuffer{};
// インスタンスインデックスバッファ
resource::ShaderResourceObj<int, instanceNum> instanceIndexBuffer{};

// パイプラインステートオブジェクト
graphics::GraphicsPipelineStateObject pso{};

// フェンス
Fence fence{};
// フェンス値
uint64_t fenceValue{};
// イベントハンドル
HANDLE waitGpuEvent{};

// コマンドリスト
CommandList commandListBegin{};
CommandList commandListDraw{};
CommandList commandListEnd{};

// コマンドキュー
CommandQueue commandQueue{};

// カメラ
DirectX::XMFLOAT3 eye(0.0f, 0.0f, -30.0f);
DirectX::XMFLOAT3 dir(0.0f, 0.0f, 1.0f);
DirectX::XMFLOAT3 up(0.0f, 1.0f, 0.0f);
float             aspect   = static_cast<float>(window::width()) / static_cast<float>(window::height());
DirectX::XMMATRIX view     = DirectX::XMMatrixLookToLH(XMLoadFloat3(&eye), XMLoadFloat3(&dir), XMLoadFloat3(&up));
DirectX::XMMATRIX proj     = DirectX::XMMatrixPerspectiveFovLH(3.14159f / 4.f, aspect, 0.1f, 1000.0f);
DirectX::XMMATRIX viewProj = view * proj;
camera::Frustum   frustum  = camera::createFrustumFromViewProjection(viewProj);

// インスタンス位置
DirectX::XMFLOAT3 position[instanceNum]{};

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
        TIME_CHECK_SCORP("フレーム");
        {
            TIME_CHECK_SCORP("更新");
            // 視錐台カリング
            auto drawCount = 0;
            for (auto i = 0; i < instanceNum; ++i) {
                if (camera::isPositionInFrustum(frustum, position[i])) {
                    instanceIndexBuffer[drawCount++] = i;
                }
            }

            // 描画開始
            commandListBegin.reset();
            frameBuffer.startRendering(commandListBegin);
            commandListBegin.get()->Close();

            // 各メッシュ描画
            commandListDraw.reset();
            frameBuffer.setToRenderTarget(commandListDraw);
            descriptorHeap.setToCommandList(commandListDraw);
            pso.setToCommandList(commandListDraw);
            mesh.setToCommandList(commandListDraw);
            sceneConstantBuffer.setToCommandList(commandListDraw, 0);
            instanceBuffer.setToCommandList(commandListDraw, 1);
            // インスタンス描画
            commandListDraw.get()->DrawIndexedInstanced(6, drawCount, 0, 0, 0);
            commandListDraw.get()->Close();

            // 描画終了
            commandListEnd.reset();
            frameBuffer.finishRendering(commandListEnd);
            commandListEnd.get()->Close();

            // コマンドリスト実行
            std::array<ID3D12CommandList*, 3> lists;

            // コマンドリスト
            lists[0] = commandListBegin.get();
            lists[1] = commandListDraw.get();
            lists[2] = commandListEnd.get();

            // コマンドリスト実行
            commandQueue.get()->ExecuteCommandLists(lists.size(), static_cast<ID3D12CommandList**>(lists.data()));
        }

        {
            SwapChain::instance().present();
            // フレームバッファのインデックスを更新する
            frameBuffer.updateBufferIndex(SwapChain::instance().currentBufferIndex());

            // GPU と CPU の同期
            fenceValue++;
            commandQueue.get()->Signal(fence.get(), fenceValue);
            if (fence.get()->GetCompletedValue() < fenceValue) {
                fence.get()->SetEventOnCompletion(fenceValue, waitGpuEvent);
                WaitForSingleObject(waitGpuEvent, INFINITE);
            }
        }
    }

    // 時間表示
    TIME_PRINT("");

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
    std::uniform_real_distribution<float> distr(0, 1);
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

            // ディスクリプタヒープを作成する
            descriptorHeap.create(DescriptorHeap::Type::CBV_SRV_UAV, 10);

            // コマンドキューを作成する
            commandQueue.create(dx12::CommandQueue::Type::Graphics);

            // スワップチェインを作成する
            SwapChain::instance().create(commandQueue, frameBuffer);

            // メッシュを作成する
            mesh.createVertexBuffer(vertexData);
            mesh.createIndexBuffer(indexData);

            // ビュー行列
            view = DirectX::XMMatrixLookToLH(XMLoadFloat3(&eye), XMLoadFloat3(&dir), XMLoadFloat3(&up));
            // プロジェクション行列
            proj = DirectX::XMMatrixPerspectiveFovLH(3.14159f / 4.f, aspect, 0.1f, 1000.0f);

            // シーンコンスタントバッファの内容を設定する
            sceneConstantBuffer.create();
            sceneConstantBuffer.createView(descriptorHeap);
            sceneConstantBuffer[0].viewProj = DirectX::XMMatrixTranspose(viewProj);

            // 描画インスタンス毎の内容を設定する
            instanceBuffer.create();
            // 描画インスタンスインデックスを設定する
            instanceIndexBuffer.create();

            // RootSignature 生成で この二つ（t0,t1）を一つの DESCRIPTOR_RANGE に纏めた為、descriptorHeap を連続で確保する
            instanceBuffer.createView(descriptorHeap);
            instanceIndexBuffer.createView(descriptorHeap);

            // 描画インスタンスの情報を初期化する
            for (auto i = 0; i < instanceNum; ++i) {
                auto rad    = distr(re) * 3.14f * 2.f;
                auto r      = distr(re) * 100.0f;
                position[i] = DirectX::XMFLOAT3(cosf(rad) * r, sinf(rad) * r, 0);

                instanceBuffer[i].world = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(position[i].x, position[i].y, 0));
                instanceBuffer[i].color = DirectX::XMFLOAT4(distr(re), distr(re), distr(re), 1.0f);
                instanceIndexBuffer[i]  = i;
            }

            // フェンス（CPUとGPUの同期オブジェクト）を作成する
            fence.create();
            waitGpuEvent = CreateEvent(nullptr, false, false, "WAIT_GPU");

            // 各コマンドリストを作成する
            commandListBegin.create();
            commandListDraw.create();
            commandListEnd.create();

            // パイプラインステートオブジェクトを作成する
            pso.create();

            // アプリケーションループ
            while (appUpdate()) {
            }

            // 終了前のGPU待ち
            fenceValue++;
            commandQueue.get()->Signal(fence.get(), fenceValue);
            fence.get()->SetEventOnCompletion(fenceValue, waitGpuEvent);
            WaitForSingleObject(waitGpuEvent, INFINITE);
            CloseHandle(waitGpuEvent);
        }
    }

    return 0;
}