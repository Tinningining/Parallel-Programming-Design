#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>

#include <arm_neon.h>

using namespace std;

vector<uint32_t> transpose_bitmap(vector<vector<uint32_t>>bitLists){
    vector<uint32_t> bitLists_transpose(bitLists[0].size()*sizeof(unsigned int)*8);
    for(int i = 0;i < bitLists_transpose.size();i ++){
        for(int j = 0;j < bitLists.size();j ++){
            bool bitSet = bitLists[j][i/(sizeof(unsigned int)*8)] & (1 << (sizeof(unsigned int)*8 - 1 - (31-i)%(sizeof(unsigned int)*8)));
            bitLists_transpose[i] = (bitLists_transpose[i] << 1) + bitSet;
        }
    }
    return bitLists_transpose;
}


vector<uint32_t> element_wise_NEON(vector<uint32_t> bitLists_transpose, int test_number) {
    int num = bitLists_transpose.size();
    vector<uint32_t> intersection(num / (sizeof(uint32_t) * 8) + 1);

    // 将测试数值复制到NEON寄存器中，以便进行逐个比较
    uint32x4_t test_number_vec = vdupq_n_u32(test_number);

    // 循环遍历输入向量并进行处理
    int i = 0;
    for (; i + 3 < bitLists_transpose.size(); i += 4) {
        // 加载4个32位整数到NEON寄存器中
        uint32x4_t transpose_val = vld1q_u32(&bitLists_transpose[i]);

        // 比较每个32位整数是否等于测试数值
        uint32x4_t equal_mask = vceqq_u32(transpose_val, test_number_vec);

        // 只保留最低位为1，其它位为0
        equal_mask = vandq_u32(equal_mask, vdupq_n_u32(1));

        // 计算当前索引在intersection中的位置
        int index = i / (sizeof(uint32_t) * 8);

        // 计算正确的位偏移量
        int bit_offset = i % (sizeof(uint32_t) * 8);

        uint32x4_t offset_val = vdupq_n_u32(pow(2,bit_offset));

        // 生成掩码进行位操作
        uint32x4_t shifted_mask = vmulq_u32(equal_mask, offset_val);

        // 将结果与intersection中的对应位置进行位或操作
        uint32x4_t intersection_val = vld1q_u32(&intersection[index]);
        intersection_val = vorrq_u32(intersection_val, shifted_mask);

        // 将结果存储回intersection中
        vst1q_u32(&intersection[index], intersection_val);
    }

    // 处理剩余元素
    for (; i < bitLists_transpose.size(); ++i) {
        if (bitLists_transpose[i] == test_number) {
            int index = i / (sizeof(uint32_t) * 8);
            intersection[index] |= (1 << (i % (sizeof(uint32_t) * 8)));
        }
    }

    return intersection;
}


vector<int> bitmap_to_int(std::vector<uint32_t> intersection){
    std::vector<int> result;
    for(int i=0;i<intersection.size();i++){
        if(intersection[i]!= 0){
            for(int j=0;j<sizeof(unsigned int)*8;j++)
                if((intersection[i]&(1<<j))!=0)
                    result.push_back(i*(sizeof(unsigned int)*8)+j);
        }
    }

    return result;
}