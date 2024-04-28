#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>

#include <emmintrin.h>//SSE
using namespace std;


/*
IntelDevcloud编译选项：g++ filename.cpp -o name -std=c++11 -O2 -march=native -mavx
*/


const int bitmap_size(vector<vector<uint32_t>>list){//位图列表长度
    vector<int> v;
    for(int i=0;i<list.size();i++)
     v.push_back(list[i].back());
    std::sort(v.begin(),v.end());
    return v.back();
}

void loadBitMap_unaligned(vector<vector<uint32_t>> &Lists,vector<vector<uint32_t>> &bitmap,int size){//加载位图列表
    for(vector<uint32_t> arr : Lists){
        vector<uint32_t> l(size,0);
        for(int num : arr){
            unsigned int bitIndex = (num)/(sizeof(uint32_t)*8);
            int offset = num%(sizeof(uint32_t)*8);
            l[bitIndex] |= (1 << offset);
        }
        bitmap.push_back(l);
    }
}

void loadBitMap_aligned(vector<vector<uint32_t>> &Lists,vector<vector<uint32_t>> &bitmap,int size){//加载位图列表
    // 计算需要的位图大小，使其为4的倍数
    int adjustedSize = ((size + 31) / 32) * 32; // 向上取整到最接近的32位整数的倍数
    for (vector<uint32_t> arr : Lists) {
        vector<uint32_t> l(adjustedSize, 0);
        for (int num : arr) {
            unsigned int bitIndex = num / 32;
            int offset = num % 32;
            l[bitIndex] |= (1 << offset);
        }
        bitmap.push_back(l);
    }
}


//***********************************listwise_Bitmap_SSE*******************************************
    
    //unaligned_SSE_Instructions
vector<uint32_t> list_wise_SSE_unaligned(std::vector<std::vector<uint32_t>>& bitmap) {

    size_t listSize = bitmap.size();
    size_t bitSize = bitmap[0].size();

    for (size_t i = 1; i < listSize; i++) {
        size_t j = 0;

        for (; j+3 < bitSize; j += 4) {
            __m128i intersec_vec = _mm_loadu_si128((__m128i*)&bitmap[0][j]);
            __m128i bitList_vec = _mm_loadu_si128((__m128i*)&bitmap[i][j]);
            intersec_vec = _mm_and_si128(intersec_vec, bitList_vec);
            _mm_storeu_si128((__m128i*)&bitmap[0][j], intersec_vec);
        }

        //处理剩余的数据
        for (; j < bitSize; j++) {
            bitmap[0][j] &= bitmap[i][j];
        }
    }

    return bitmap[0];
}

    //aligned_SSE_Instructions
vector<uint32_t> list_wise_SSE_aligned(std::vector<std::vector<uint32_t>>& bitmap) {

    size_t listSize = bitmap.size();
    size_t bitSize = bitmap[0].size();

    for (size_t i = 1; i < listSize; i++) {
        size_t j = 0;

        for (; j < bitSize; j += 4) {
            __m128i intersec_vec = _mm_load_si128((__m128i*)&bitmap[0][j]);
            __m128i bitList_vec = _mm_load_si128((__m128i*)&bitmap[i][j]);
            intersec_vec = _mm_and_si128(intersec_vec, bitList_vec);
            _mm_storeu_si128((__m128i*)&bitmap[0][j], intersec_vec);
        }

    }
    return bitmap[0];
}


vector<uint32_t> bitmap_to_int(std::vector<uint32_t> intersection){
    std::vector<uint32_t> result;
    for(int i=0;i<intersection.size();i++){
        if(intersection[i]!= 0){
            for(int j=0;j<sizeof(unsigned int)*8;j++)
                if((intersection[i]&(1<<j))!=0)
                    result.push_back(i*(sizeof(unsigned int)*8)+j);
        }
    }
    return result;
}
