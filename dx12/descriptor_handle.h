#pragma once

#include "dx12/device.h"

namespace dx12 {

//---------------------------------------------------------------------------------
/**
 * @brief	ディスクリプタハンドル
 */
struct DescriptorHandle {
    uint32_t                    index_{};
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_{};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_{};
    uint32_t                    incrementSize_{};
};

}  // namespace dx12
