#include <iostream>
#include <omp.h>
#include <vector>
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

vector<uint32_t> transpose_bitmap_openmp_dynamic(vector<vector<uint32_t>>bitLists){
    vector<uint32_t> bitLists_transpose(bitLists[0].size()*sizeof(unsigned int)*8);

    int N = bitLists_transpose.size();

    #pragma omp parallel num_threads(NUM_THREADS)
    {
        #pragma omp for schedule(dynamic)
        for(int k = 0;k < NUM_THREADS;k ++){
            int istart,iend;
            istart = k * N / NUM_THREADS;
            iend = (k + 1) * N / NUM_THREADS;
            if(k == NUM_THREADS - 1) iend = N;

            for(int i = istart; i < iend; i++) {
                for(int j = 0;j < bitLists.size();j ++){
                    bool bitSet = bitLists[j][i/(sizeof(unsigned int)*8)] & (1 << (sizeof(unsigned int)*8 - 1 - (31-i)%(sizeof(unsigned int)*8)));
                    bitLists_transpose[i] = (bitLists_transpose[i] << 1) + bitSet;
                }
            }
        }
    }

    return bitLists_transpose;
}

vector<int> element_wise_bitmap_openmp_dynamic(vector<uint32_t>bitLists_transpose,int test_number){
    vector<int> result;

    int N = bitLists_transpose.size();
    vector<vector<int>> result_part(NUM_THREADS);

    #pragma omp parallel num_threads(NUM_THREADS)
    {
        #pragma omp for schedule(dynamic)
        for(int k = 0;k < NUM_THREADS;k ++){
            int istart,iend;
            istart = k * N / NUM_THREADS;
            iend = (k + 1) * N / NUM_THREADS;
            if(k == NUM_THREADS - 1) iend = N;

            for(int i = istart; i < iend; i++) {
                if(bitLists_transpose[i] == test_number)
                    result_part[k].push_back(i);
            }
        }
    }

    for(int i = 0;i < NUM_THREADS;i++)
        result.insert(result.end(), result_part[i].begin(), result_part[i].end());
    return result;
}