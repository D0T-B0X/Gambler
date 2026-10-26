#include <hip/hip_runtime.h>
#include <cstdint>

#include "parallel_search.hpp"

__global__ void parallel_search(
    const uint32_t* __restrict__ d_arr, 
    uint32_t key, 
    size_t test_size, 
    uint* d_found, 
    size_t* d_result_index
) {
    size_t i = static_cast<size_t>(blockIdx.x) * blockDim.x + threadIdx.x;

    if (i >= test_size) 
        return;

    if (d_arr[i] == key) 
        if (atomicCAS(d_found, 0, 1) == 0) 
            *d_result_index = i;
}   

void launch_parallel_search(const uint32_t* d_arr,
                            uint32_t target_key,
                            size_t test_size,
                            unsigned int* d_found_flag,
                            size_t* d_result_index)
{
    constexpr int threads_per_block = 256;
    int num_blocks = static_cast<int>((test_size + threads_per_block - 1) / threads_per_block);

    hipLaunchKernelGGL(parallel_search, 
                       dim3(num_blocks), 
                       dim3(threads_per_block), 
                       0, 0, 
                       d_arr, target_key, test_size, d_found_flag, d_result_index);
}