#include <iostream>
#include <omp.h>
#include <vector>
#include <algorithm>
using namespace std;

const int NUM_THREADS = 4;

bool binarySearch(const vector<uint32_t> list, int element) {
    int left = 0;
    int right = list.size() - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (list[mid] == element) {
            return true;
        } else if (list[mid] < element) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return false;
}

std::vector<uint32_t> transpose_bitmap_openmp_static(const std::vector<std::vector<uint32_t>>& bitLists) {
    std::vector<uint32_t> bitLists_transpose(bitLists[0].size() * sizeof(unsigned int) * 8);

    int bit_count = sizeof(unsigned int) * 8;

    #pragma omp parallel for num_threads(NUM_THREADS) schedule(static)
    for (int i = 0; i < bitLists_transpose.size(); i++) {
        uint32_t transposed_value = 0;
        for (int j = 0; j < bitLists.size(); j++) {
            bool bitSet = bitLists[j][i / bit_count] & (1 << (bit_count - 1 - (i % bit_count)));
            transposed_value = (transposed_value << 1) | bitSet;
        }
        bitLists_transpose[i] = transposed_value;
    }

    return bitLists_transpose;
}


std::vector<int> element_wise_bitmap_openmp_static(const std::vector<uint32_t>& bitLists_transpose, int test_number) {
    std::vector<int> result;

    #pragma omp parallel num_threads(NUM_THREADS)
        #pragma omp for schedule(static) nowait
        for (int i = 0; i < bitLists_transpose.size(); i++) {
            if (bitLists_transpose[i] == test_number) {
                #pragma omp critical
                result.push_back(i);
            }
        }
    sort(result.begin(), result.end());
    return result;
}