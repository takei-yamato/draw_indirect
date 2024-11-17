
#include "window/window.h"

#include "utility/time_counter.h"

#include "dx12/command_queue.h"
#include "dx12/command_list.h"
#include "dx12/descriptor_heap.h"
#include "dx12/device.h"
#include "dx12/swap_chain.h"
#include "dx12/fence.h"
#include "dx12/resource/shader_resource.h"
#include "dx12/resource/unordered_access.h"
#include "dx12/resource/constant.h"

#include "dx12/graphics/graphics_pipeline_state_object.h"
#include "dx12/graphics/compute_pipeline_state_object.h"

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

constexpr uint32_t frameBufferNum = 2;                  // フレームバッファ数
constexpr uint32_t instanceNum    = 8 * 8 * 8 * 8 * 8;  // インスタンス数（同一の物を一度に描画する数）

// シーンコンスタントバッファのフォーマット
struct ConstantBufferFormat {
    DirectX::XMMATRIX viewProj{};  // ビュープロジェクション
};

// インスタンス情報バッファのフォーマット
struct InstanceBufferFormat {
    DirectX::XMMATRIX world{};  // オブジェクトのワールド座標
    DirectX::XMFLOAT4 color{};  // オブジェクトのカラー};
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
uint16_t indexData[] = {
    0, 1, 2,
    2, 1, 3};

resource::FrameBuffer frameBuffer(frameBufferNum);  // フレームバッファ
DescriptorHeap        descriptorHeap{};             // ディスクリプタヒープ
resource::Mesh        mesh{};                       // メッシュ

resource::ConstantBufferObj<ConstantBufferFormat>              sceneData{};          // シーンデータ
resource::ShaderResourceObj<InstanceBufferFormat, instanceNum> instanceData{};       // 各インスタンスデータ
resource::ConstantBufferObj<camera::Frustum>                   frustumData{};        // フラスタムデータ
resource::UnorderedAccessObj<int>                              drawInstanceCount{};  // 描画するインスタンスのカウント
resource::UnorderedAccessObj<int, instanceNum>                 drawInstanceIndex{};  // 描画するインスタンスのインデックス（同一リソースで SRV UAV の両方を作る）

graphics::GraphicsPipelineStateObject graphicsPso{};  // グラフィックスパイプラインステートオブジェクト
graphics::ComputePipelineStateObject  computePso{};   // コンピュートパイプラインステートオブジェクト

Fence    fence{};         // コマンドフェンス
uint64_t fenceValue{};    // フェンス値
HANDLE   waitGpuEvent{};  // GPU と CPU 同期用のイベントハンドル

CommandList commandListBegin{};    // 開始処理用コマンドリスト
CommandList commandListDraw{};     // 描画用コマンドリスト
CommandList commandListCompute{};  // コンピュート用コマンドリスト
CommandList commandListEnd{};      // 終了処理用コマンドリスト

CommandQueue commandQueue{};         // 描画用コマンドキュー
CommandQueue commandQueueCompute{};  // コンピュート用コマンドキュー

DirectX::XMFLOAT3 position[instanceNum]{};  // インスタンス位置

// カメラ
const DirectX::XMFLOAT3 eye(0.0f, 0.0f, -40.0f);
const DirectX::XMFLOAT3 dir(0.0f, 0.0f, 1.0f);
const DirectX::XMFLOAT3 up(0.0f, 1.0f, 0.0f);
const float             aspect   = static_cast<float>(window::width()) / static_cast<float>(window::height());
const DirectX::XMMATRIX view     = DirectX::XMMatrixLookToLH(XMLoadFloat3(&eye), XMLoadFloat3(&dir), XMLoadFloat3(&up));
const DirectX::XMMATRIX proj     = DirectX::XMMatrixPerspectiveFovLH(3.14159f / 4.f, aspect, 0.1f, 1000.0f);
const DirectX::XMMATRIX viewProj = view * proj;

// フラスタム（カリングされている事が分かるように狭める）
const DirectX::XMMATRIX culProj = DirectX::XMMatrixPerspectiveFovLH(3.14159f / 4.5f, aspect, 0.1f, 1000.0f);
camera::Frustum         frustum = camera::createFrustumFromViewProjection(view * culProj);

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

			// 1 が入力されていたら GPU カリングを利用する
            auto useGpuCulling = input::Input::instance().getKey('1');

            // 描画するインスタンス数
            auto drawCount = 0;

            if (useGpuCulling) {
                // コンピュート処理による視錐台カリング
                // コンピュートコマンド作成
                commandListCompute.reset();
                computePso.setToCommandList(commandListCompute);
                descriptorHeap.setToCommandList(commandListCompute);

                // 計算に必要な情報を設定
                sceneData.setToCommandList(commandListCompute, ViewType::CBV, 0);
                frustumData.setToCommandList(commandListCompute, ViewType::CBV, 1);
                instanceData.setToCommandList(commandListCompute, ViewType::SRV, 2);
                drawInstanceIndex.setToCommandList(commandListCompute, ViewType::UAV, 3);
                drawInstanceCount.setToCommandList(commandListCompute, ViewType::UAV, 4);

                // 計算開始
                commandListCompute.get()->Dispatch(8, 8, 8);
				// リソース書き込み完了のバリア
                drawInstanceCount.resourceBarrier(commandListCompute, D3D12_RESOURCE_BARRIER_TYPE_UAV);

				commandListCompute.get()->Close();

                // コマンドリスト実行
                std::array<ID3D12CommandList*, 1> lists{commandListCompute.get()};
                commandQueueCompute.get()->ExecuteCommandLists(lists.size(), static_cast<ID3D12CommandList**>(lists.data()));

                // 計算結果（描画インスタンスのカウント）を CPU で利用する為、GPU と CPU を同期させなければならない
                fenceValue++;
                commandQueueCompute.get()->Signal(fence.get(), fenceValue);
                if (fence.get()->GetCompletedValue() < fenceValue) {
                    fence.get()->SetEventOnCompletion(fenceValue, waitGpuEvent);
                    WaitForSingleObject(waitGpuEvent, INFINITE);
                }

                // 計算結果を取得する
                drawInstanceCount.map();
                drawCount            = drawInstanceCount[0];
                drawInstanceCount[0] = 0;
                drawInstanceCount.unmap();

            } else {
                drawInstanceIndex.map();
                // CPU による視錐台カリング
                for (auto i = 0; i < instanceNum; ++i) {
                    if (camera::isPositionInFrustum(frustum, position[i])) {
                        drawInstanceIndex[drawCount++] = i;
                    }
                }
                drawInstanceIndex.unmap();
            }

            // 描画処理
            {
                // 描画開始
                commandListBegin.reset();
                frameBuffer.startRendering(commandListBegin);
                commandListBegin.get()->Close();

                // 各メッシュ描画
                commandListDraw.reset();
                frameBuffer.setToRenderTarget(commandListDraw);
                descriptorHeap.setToCommandList(commandListDraw);
                graphicsPso.setToCommandList(commandListDraw);
                mesh.setToCommandList(commandListDraw);

                // メッシュ描画に必要な情報を設定
                sceneData.setToCommandList(commandListDraw, ViewType::CBV, 0);
                instanceData.setToCommandList(commandListDraw, ViewType::SRV, 1);
                drawInstanceIndex.setToCommandList(commandListDraw, ViewType::SRV, 2);

                // SRVとしてアクセスできるようにバリア
                drawInstanceIndex.resourceBarrier(commandListDraw, D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

                // インスタンス描画
                commandListDraw.get()->DrawIndexedInstanced(6, drawCount, 0, 0, 0);

                // UAV としてアクセスできるようにバリア
                drawInstanceIndex.resourceBarrier(commandListDraw, D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

                commandListDraw.get()->Close();

                // 描画終了
                commandListEnd.reset();
                frameBuffer.finishRendering(commandListEnd);
                commandListEnd.get()->Close();

                // コマンドリスト実行
                std::array<ID3D12CommandList*, 3> lists;
                lists[0] = commandListBegin.get();
                lists[1] = commandListDraw.get();
                lists[2] = commandListEnd.get();
                commandQueue.get()->ExecuteCommandLists(lists.size(), static_cast<ID3D12CommandList**>(lists.data()));
            }
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
            descriptorHeap.create(DescriptorHeap::Type::CBV_SRV_UAV, 16);

            // コマンドキューを作成する
            commandQueue.create(dx12::CommandType::Graphics);
            commandQueueCompute.create(dx12::CommandType::Compute);

            // スワップチェインを作成する
            SwapChain::instance().create(commandQueue, frameBuffer);

            // メッシュを作成する
            mesh.createVertexBuffer(vertexData);
            mesh.createIndexBuffer(indexData);

            // シーンデータ用リソース
            sceneData.create();
            sceneData[0].viewProj = DirectX::XMMatrixTranspose(viewProj);

            // 描画インスタンスデータ用リソース
            instanceData.create();
            // 描画インスタンスの情報を初期化する
            for (auto i = 0; i < instanceNum; ++i) {
                auto rad              = distr(re) * 3.14f * 2.f;
                auto r                = distr(re) * 100.0f;
                position[i]           = DirectX::XMFLOAT3(cosf(rad) * r, sinf(rad) * r, distr(re) * 20.0f);
                instanceData[i].world = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(position[i].x, position[i].y, position[i].z));
                instanceData[i].color = DirectX::XMFLOAT4(distr(re), distr(re), distr(re), 1.0f);
            }

            // フラスタムデータ用リソース
            frustumData.create();
            frustumData[0] = frustum;

            // 描画するインスタンスのインデックス用リソース
            drawInstanceIndex.create();
            // 描画するインスタンスのカウント用リソース
            drawInstanceCount.create();

            // 各リソースのビューを生成する
            sceneData.createView(ViewType::CBV, descriptorHeap);
            frustumData.createView(ViewType::CBV, descriptorHeap);
            instanceData.createView(ViewType::SRV, descriptorHeap);
            drawInstanceCount.createView(ViewType::UAV, descriptorHeap);
            drawInstanceIndex.createView(ViewType::UAV, descriptorHeap);
            drawInstanceIndex.createView(ViewType::SRV, descriptorHeap);

            // フェンス（CPUとGPUの同期オブジェクト）を作成する
            fence.create();
            waitGpuEvent = CreateEvent(nullptr, false, false, "WAIT_GPU");

            // 各コマンドリストを作成する
            commandListBegin.create(CommandType::Graphics);
            commandListDraw.create(CommandType::Graphics);
            commandListEnd.create(CommandType::Graphics);
            commandListCompute.create(CommandType::Compute);

            // パイプラインステートオブジェクトを作成する
            graphicsPso.create();
            computePso.create();

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