#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>

#include <immintrin.h>//AVX&AVX-512
using namespace std;


/*
IntelDevcloud_AVX编译选项：g++ -std=c++11 filename.cpp -o name -march=native -mavx -O2
IntelDevcloud_AVX-512编译选项：g++ -std=c++11 filename.cpp -o name -march=native -mavx512f -O2
*/


const int bitmap_size(vector<vector<uint32_t>>list){//位图列表长度
    vector<int> v;
    for(int i=0;i<list.size();i++)
     v.push_back(list[i].back());
    std::sort(v.begin(),v.end());
    return v.back();
}

void loadBitMap(vector<vector<uint32_t>> &Lists,vector<vector<uint32_t>> &bitmap,int size){//加载位图列表
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

//***********************************listwise_Bitmap_AVX*******************************************
vector<uint32_t> list_wise_AVX(std::vector<std::vector<uint32_t>>& bitmap) {

    // 使用AVX并行化
    size_t listSize = bitmap.size();
    size_t bitSize = bitmap[0].size();

    // 处理bitmap中的数据
    for (size_t i = 1; i < listSize; i++) {
        size_t j = 0;

        // 使用AVX并行化处理
        for (; j< bitSize; j += 8) {
            __m256i intersec_vec = _mm256_load_si256((__m256i*)&bitmap[0][j]);
            __m256i bitList_vec = _mm256_load_si256((__m256i*)&bitmap[i][j]);
            intersec_vec = _mm256_and_si256(intersec_vec, bitList_vec);
            _mm256_store_si256((__m256i*)&bitmap[0][j], intersec_vec);
        }

        // // 处理剩余的数据
        // for (; j < bitSize; j++) {
        //     bitmap[0][j] &= bitmap[i][j];
        // }
    }

    return bitmap[0];
}

//*********************************listwise_Bitmap_AVX-512*****************************************
vector<uint32_t> list_wise_AVX512(std::vector<std::vector<uint32_t>>& bitmap) {
    // 使用AVX-512并行化
    size_t listSize = bitmap.size();
    size_t bitSize = bitmap[0].size();

    // 处理 bitmap 中的数据
    for (size_t i = 1; i < listSize; i++) {
        size_t j = 0;

        // 使用AVX-512并行化处理
        for (; j + 15 < bitSize; j += 16) {
            // 加载16个 uint32_t 元素到 __m512i 向量中
            __m512i intersec_vec = _mm512_loadu_si512(&bitmap[0][j]);
            __m512i bitList_vec = _mm512_loadu_si512(&bitmap[i][j]);

            // 执行按位与操作
            intersec_vec = _mm512_and_si512(intersec_vec, bitList_vec);

            // 存储结果
            _mm512_storeu_si512(&bitmap[0][j], intersec_vec);
        }

        // 处理剩余的数据
        for (; j < bitSize; j++) {
            bitmap[0][j] &= bitmap[i][j];
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