#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <print>
#include <time.h>
#include <hip/hip_runtime_api.h>

#include "csv_reader.hpp"
#include "parallel_search.hpp"

constexpr uint32_t MAX_TEST_SIZE = 1e9;

// bullshit to stop hipcc from optimizing the actual test away
template <class T>
__attribute__((always_inline)) inline void DoNotOptimize(const T& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}

class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    std::chrono::duration<double> duration;

public:
    Timer() {
        start = std::chrono::high_resolution_clock::now();
    }

    ~Timer() {
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
        double ms = duration.count() * 1000.0f;
        std::println("{:.6f}", ms);
    }
};

class Search {
private:
    std::vector<uint32_t> arr;

public:
    Search() = default;

    void load_tests(std::string path) {
        arr.reserve(MAX_TEST_SIZE);
        CSVReader::read_csv(path, arr);
    }

    void las_vegas(size_t key_i, size_t test_size) {
        Timer timer;

        uint32_t key = arr[key_i];
        while (true) {
            size_t i = rand() % test_size;
            DoNotOptimize(arr[i]);
            if (key == arr[i]) 
                return;
        }
    }

    void las_vegas_bitmap(size_t key_i, size_t test_size) {
        Timer timer;
        std::vector<bool> visited_bitmap(MAX_TEST_SIZE, false);

        uint32_t key = arr[key_i];
        while (true) {
            size_t i = rand() % test_size;
            
            DoNotOptimize(arr[i]);

            // skip check if already visited
            if (visited_bitmap[i]) 
                continue;

            visited_bitmap[i] = true;
            if (key == arr[i]) 
                return;
        }
    }

    void linear(size_t key_i, size_t test_size) {
        Timer timer;

        uint32_t key = arr[key_i];
        for (size_t i = 0; i < test_size; ++i) {
            DoNotOptimize(arr[i]);
            if (i != key_i && arr[i] == key)
                return;
        }
    }

    void parallel_search(size_t key_i, size_t test_size) {
        uint32_t* d_arr = nullptr;
        uint* d_found = nullptr;
        size_t* d_result_index = nullptr;

        hipError_t _ = hipMalloc(&d_arr, sizeof(uint32_t) * MAX_TEST_SIZE);
        _ = hipMalloc(&d_found, sizeof(uint));
        _ = hipMalloc(&d_result_index, sizeof(size_t));
        
        _ = hipMemcpy(d_arr, arr.data(), sizeof(uint32_t) * MAX_TEST_SIZE, hipMemcpyHostToDevice);
        uint h_flag = 0;
        size_t h_idx = -1;
        _ = hipMemcpy(d_found, &h_flag, sizeof(uint), hipMemcpyHostToDevice);
        _ = hipMemcpy(d_result_index, &h_idx, sizeof(size_t), hipMemcpyHostToDevice);

        uint32_t key = arr[key_i];

        // fuckass cpp rules, I'm not doing Timer* timer = new Timer(); delete timer; deranged bullshit
        {
            Timer timer;

            launch_parallel_search(d_arr, key, test_size, d_found, d_result_index);
            _ = hipDeviceSynchronize();
        }

        _ = hipFree(d_arr);
        _ = hipFree(d_found);
        _ = hipFree(d_result_index);
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::println("1 argument expected, {} found", argc - 1);
        std::exit(-1);
    }

    char type = argv[1][0];
     
    Search search;
    search.load_tests("tests.csv");

    for(int i = 100; i <= MAX_TEST_SIZE; i *= 10) {
        for (int j = 0; j < 10; ++j) {
            size_t key_i = rand() % i;
        
            switch (type) {
                case 'p':
                    search.parallel_search(key_i, i);
                    break;
                case 'v':
                    search.las_vegas(key_i, i);
                    break;
                case 'b':
                    search.las_vegas_bitmap(key_i, i);
                    break;
                case 'l':
                    search.linear(key_i, i);
                    break;
                default:
                    std::println("Invalid search type");
                    exit(-1);
            }

        }
    }
}