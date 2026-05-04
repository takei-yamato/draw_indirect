
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

#include "dx12/graphics/graphics_root_signature.h"
#include "dx12/graphics/compute_root_signature.h"

#include "dx12/graphics/command_signature.h"

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

// ExecuteIndirect 用引数バッファのフォーマット
struct IndirectArgs {
    D3D12_GPU_VIRTUAL_ADDRESS    drawInstanceIndexes{};  // 描画するインスタンスのインデックスリストの GPU 仮想アドレス
    D3D12_DRAW_INDEXED_ARGUMENTS args{};                 // DrawIndexedInstanced の引数
};

// 頂点データ
Vertex vertexData[] = {
    { {-1.0f, 1.0f, 0.0f}, {0, 0}},
    {  {1.0f, 1.0f, 0.0f}, {1, 1}},
    {{-1.0f, -1.0f, 0.0f}, {0, 1}},
    { {1.0f, -1.0f, 0.0f}, {1, 0}}
};

// インデックスデータ
uint16_t indexData[] = {
    0, 1, 2,
    2, 1, 3};

resource::FrameBuffer frameBuffer(frameBufferNum);  // フレームバッファ
DescriptorHeap        descriptorHeap{};             // ディスクリプタヒープ
resource::Mesh        mesh{};                       // メッシュ

resource::ConstantBufferObj<ConstantBufferFormat>              sceneData{};            // シーンデータ
resource::ShaderResourceObj<InstanceBufferFormat, instanceNum> instanceData{};         // 各インスタンスデータ
resource::ConstantBufferObj<camera::Frustum>                   frustumData{};          // フラスタムデータ
resource::UnorderedAccessObj<int, instanceNum>                 drawInstanceIndexes{};  // 描画するインスタンスのインデックスリスト（UAV のみ作成する）
resource::UnorderedAccessObj<IndirectArgs>                     indirectArgs{};         // ExecuteIndirect 用引数バッファ

graphics::GraphicsRootSignature graphicsRootSignature{};  // グラフィックスルートシグネチャ
graphics::ComputeRootSignature  computeRootSignature{};   // コンピュートルートシグネチャ
graphics::CommandSignature      drawCommandSignature{};   // ExecuteIndirect 用コマンドシグネチャ

graphics::GraphicsPipelineStateObject graphicsPso{};  // グラフィックスパイプラインステートオブジェクト
graphics::ComputePipelineStateObject  computePso{};   // コンピュートパイプラインステートオブジェクト

graphics::Shader drawShader{};     // インスタンス描画シェーダ
graphics::Shader computeShader{};  // コンピュートシェーダ

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
const DirectX::XMFLOAT3 eye(0.0f, 0.0f, -100.0f);
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

            // 1 が入力されていたら CPU でカリングする
            auto useCpuCulling = input::Input::instance().getKey('1');

            if (!useCpuCulling) {
                // GPU による視錐台カリング

                // 前フレームのフェンス保証のもと、instanceCount を GPU 実行前にリセット
                indirectArgs.map();
                indirectArgs->args.InstanceCount = 0;

                // コンピュートコマンド作成
                commandListCompute.reset();
                computePso.setToCommandList(commandListCompute);
                descriptorHeap.setToCommandList(commandListCompute);

                // 計算に必要な情報を設定
                sceneData.setToCommandList(commandListCompute, ViewType::CBV, 0);
                frustumData.setToCommandList(commandListCompute, ViewType::CBV, 1);
                instanceData.setToCommandList(commandListCompute, ViewType::SRV, 2);
                drawInstanceIndexes.setToCommandList(commandListCompute, ViewType::UAV, 3);
                indirectArgs.setToCommandList(commandListCompute, ViewType::UAV, 4);

                // 計算開始
                commandListCompute.get()->Dispatch(8, 8, 8);
                // indirectArgs への書き込み完了バリア
                indirectArgs.resourceBarrier(commandListCompute, D3D12_RESOURCE_BARRIER_TYPE_UAV);

                commandListCompute.get()->Close();

                // コンピュートコマンドリスト実行
                std::array<ID3D12CommandList*, 1> computeLists{commandListCompute.get()};
                commandQueueCompute.get()->ExecuteCommandLists(computeLists.size(), static_cast<ID3D12CommandList**>(computeLists.data()));

                // コンピュートキューの完了をフェンスでシグナル
                fenceValue++;
                commandQueueCompute.get()->Signal(fence.get(), fenceValue);

                // グラフィクスキューを GPU サイドでコンピュート完了まで待機させる（CPU ブロックなし）
                commandQueue.get()->Wait(fence.get(), fenceValue);

            } else {
                // CPU による視錐台カリング
                auto drawCount = 0;
                drawInstanceIndexes.map();
                for (auto i = 0; i < instanceNum; ++i) {
                    if (camera::isPositionInFrustum(frustum, position[i])) {
                        drawInstanceIndexes[drawCount++] = i;
                    }
                }
                drawInstanceIndexes.unmap();

                // 引数バッファを CPU から直接書き込む
                indirectArgs.map();
                *indirectArgs = {drawInstanceIndexes.resource()->GetGPUVirtualAddress(), 6, static_cast<uint32_t>(drawCount), 0, 0, 0};
                indirectArgs.unmap();
            }

            // 描画処理（GPU・CPU カリング共通）
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

                // indirectArgs のリソースバリア( UAV → INDIRECT_ARGUMENT )
                indirectArgs.resourceBarrier(commandListDraw, D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
                // GPU が生成した引数バッファを使ってインスタンス描画
                commandListDraw.get()->ExecuteIndirect(drawCommandSignature.get(), 1, indirectArgs.resource(), 0, nullptr, 0);
                // 次フレームの為に indirectArgs のリソースバリア( INDIRECT_ARGUMENT -> UAV)
                indirectArgs.resourceBarrier(commandListDraw, D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

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
    std::uniform_real_distribution<float> distr(0.0f, 1.0f);
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
            sceneData->viewProj = DirectX::XMMatrixTranspose(viewProj);

            // 描画インスタンスデータ用リソース
            instanceData.create();
            // 描画インスタンスの情報を初期化する
            for (auto i = 0; i < instanceNum; ++i) {
                auto rad              = distr(re) * 3.14f * 2.f;
                auto r                = distr(re) * 200.0f;
                position[i]           = DirectX::XMFLOAT3(cosf(rad) * r, sinf(rad) * r, (distr(re) - 0.5f) * 2.0f * 50.0f);
                instanceData[i].world = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(position[i].x, position[i].y, position[i].z));
                instanceData[i].color = DirectX::XMFLOAT4(distr(re), distr(re), distr(re), 1.0f);
            }

            // フラスタムデータ用リソース
            frustumData.create();
            *frustumData = frustum;

            // 描画するインスタンスのインデックス用リソース
            drawInstanceIndexes.create();

            // ExecuteIndirect 用引数バッファ用リソース
            indirectArgs.create();
            *indirectArgs = {drawInstanceIndexes.resource()->GetGPUVirtualAddress(), 6, 0, 0, 0, 0};

            // 各リソースのビューを生成する
            sceneData.createView(ViewType::CBV, descriptorHeap);
            frustumData.createView(ViewType::CBV, descriptorHeap);
            instanceData.createView(ViewType::SRV, descriptorHeap);
            indirectArgs.createView(ViewType::UAV, descriptorHeap);
            drawInstanceIndexes.createView(ViewType::UAV, descriptorHeap);

            // フェンス（CPUとGPUの同期オブジェクト）を作成する
            fence.create();
            waitGpuEvent = CreateEvent(nullptr, false, false, "WAIT_GPU");

            // 各コマンドリストを作成する
            commandListBegin.create(CommandType::Graphics);
            commandListDraw.create(CommandType::Graphics);
            commandListEnd.create(CommandType::Graphics);
            commandListCompute.create(CommandType::Compute);

            // ルートシグネチャを作成する
            graphicsRootSignature.create();
            computeRootSignature.create();

            // ExecuteIndirect 用コマンドシグネチャを作成する
            drawCommandSignature.create(&graphicsRootSignature, sizeof(IndirectArgs));

            // シェーダを作成する
            drawShader.create("asset/color_instance.hlsl");
            computeShader.create("asset/calc.hlsl");

            // パイプラインステートオブジェクトを作成する
            graphicsPso.create(&graphicsRootSignature, &drawShader);
            computePso.create(&computeRootSignature, &computeShader);

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