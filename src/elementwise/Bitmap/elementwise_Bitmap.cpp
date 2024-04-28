#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>

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


vector<uint32_t> element_wise_bitmap(vector<uint32_t>bitLists_transpose,int test_number){
    int num = bitLists_transpose.size();
    vector<uint32_t> intersection(num/(sizeof(uint32_t)*8)+1);
    for(int i = 0;i < bitLists_transpose.size();i ++){
        if(bitLists_transpose[i] == test_number){
            int offset = i%(sizeof(uint32_t)*8);
            intersection[i/(sizeof(uint32_t)*8)] |= (1 << offset);
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