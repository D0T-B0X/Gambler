#ifndef PARALLEL_SEARCH_H
#define PARALLEL_SEARCH_H

#include <cstdint>
#include <cstddef>

void launch_parallel_search(
    const uint32_t* d_arr,
    uint32_t key,
    size_t test_size,
    unsigned int* d_found,
    size_t* d_result_index
);

#endif