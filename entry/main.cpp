
#include "window/window.h"

#include "utility/time_counter.h"

#include "dx12/command_queue.h"
#include "dx12/command_list.h"
#include "dx12/descriptor_heap.h"
#include "dx12/device.h"
#include "dx12/swap_chain.h"
#include "dx12/fence.h"
#include "dx12/resource/unordered_access.h"
#include "dx12/resource/shader_resource.h"

#include "dx12/graphics/compute_pipeline_state_object.h"

#include "dx12/resource/mesh.h"
#include "dx12/resource/frame_buffer.h"

#include "input/input.h"

#include <array>
#include <random>
#include <ppl.h>

using namespace dx12;

namespace {

// フレームバッファ(二つ分)
resource::FrameBuffer frameBuffer(2);

// ディスクリプタヒープ
DescriptorHeap descriptorHeap{};

// アンオーダードアクセス
resource::UnorderedAccessObj<int, 256> unorderedAccess{};
// シェーダリソース
resource::ShaderResourceObj<int, 256> shaderResource{};

// パイプラインステートオブジェクト
graphics::ComputePipelineStateObject pso{};

// コマンドキュー
CommandQueue commandQueue{};
CommandQueue commandQueueCompute{};

// フェンス
Fence fence{};

// コマンドリスト
CommandList commandListCompute{};
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

    // シェーダーリソースを更新する
    static auto currentNum = 0;
    bool        displayOnConsole{};
    {
        auto updateResource = [&](auto mul) {
            for (auto i = 0; i < 256; ++i) {
                shaderResource[i] = i * mul;
            }
        };

        auto temp = currentNum;
        if (input::Input::instance().getKey('1')) {
            currentNum = 1;
        } else if (input::Input::instance().getKey('2')) {
            currentNum = 2;
        }

        updateResource(currentNum);

        // リソースが更新された時だけコンソールに表示する
        displayOnConsole = currentNum != temp;
    }

    // コンピュートコマンド作成
    {
        commandListCompute.reset();

        pso.setToCommandList(commandListCompute);

        descriptorHeap.setToCommandList(commandListCompute);

        shaderResource.setToCommandList(commandListCompute, 0);
        unorderedAccess.setToCommandList(commandListCompute, 0);

        commandListCompute.get()->Dispatch(4, 4, 1);
        commandListCompute.get()->Close();
    }

    // コマンドリスト実行
    {
        // 描画開始コマンドリスト
        std::array<ID3D12CommandList*, 1> lists{ commandListCompute.get() };

        // コマンドリスト実行
        commandQueueCompute.get()->ExecuteCommandLists(lists.size(), static_cast<ID3D12CommandList**>(lists.data()));
    }

    // フェンス設定
    {
        fence.get()->Signal(0);
        commandQueueCompute.get()->Signal(fence.get(), 1);
    }

    // GPU処理が全て終了するまでCPUを待たせる
    {
        auto event = CreateEvent(nullptr, false, false, "WAIT_GPU");
        fence.get()->SetEventOnCompletion(1, event);
        WaitForSingleObject(event, INFINITE);
        CloseHandle(event);
    }

    // 結果をコンソールに表示する
    if (displayOnConsole) {
        TRACE("!!!!! unorderedAccess の内容が更新されました !!!!!");
        for (auto i = 0; i < 256; ++i) {
            auto ans = unorderedAccess[i];
            TRACE("%d", ans);
        }
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

            // ディスクリプタヒープを作成する
            descriptorHeap.create(DescriptorHeap::Type::CBV_SRV_UAV, 10);

            // コマンドキューを作成する
            commandQueue.create(CommandQueue::Type::Graphics);
            commandQueueCompute.create(CommandQueue::Type::Compute);

            // スワップチェインを作成する
            SwapChain::instance().create(commandQueue, frameBuffer);

            // リソースとビューを作成する
            shaderResource.create();
            shaderResource.createView(descriptorHeap);
            unorderedAccess.create();
			unorderedAccess.createView(descriptorHeap);

            // フェンス（CPUとGPUの同期オブジェクト）を作成する
            fence.create();

            // コマンドリストを作成する
            commandListCompute.createCompute();

            // パイプラインステートオブジェクトを作成する
            pso.create();

            // アプリケーションループ
            while (appUpdate()) {
            }
        }
    }

    return 0;
}
