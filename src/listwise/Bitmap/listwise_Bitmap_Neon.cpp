#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>

#include <arm_neon.h>

using namespace std;


/*
鲲鹏服务器编译选项：g++ -std=c++11 filename.cpp -o name -march=armv8-a -O2
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

//**********************************listwise_Bitmap_NEON*******************************************
vector<uint32_t> list_wise_NEON(vector<vector<uint32_t>>bitmap){
    
    size_t listSize = bitmap.size();
    size_t bitSize = bitmap[0].size();
    
    for (size_t i = 1; i < listSize; i++) {
        size_t j = 0;
        
        for (; j+3 < bitSize; j += 4) {
            uint32x4_t intersec_vec = vld1q_u32(&bitmap[0][j]);
            uint32x4_t bitList_vec = vld1q_u32(&bitmap[i][j]);
            intersec_vec = vandq_u32(intersec_vec, bitList_vec);
            vst1q_u32(&bitmap[0][j], intersec_vec);
        }
    
        //处理剩余的数据
        for (; j < bitSize; j++)
            bitmap[0][j] &= bitmap[i][j];  
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
