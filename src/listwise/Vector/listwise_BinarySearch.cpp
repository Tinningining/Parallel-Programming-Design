#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
using namespace std;

//***************************************listwise_BinarySearch*******************************************
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

vector<uint32_t> list_wise(vector<vector<uint32_t>> &Lists){//按表求交
    for(int i=1;i<Lists.size();i++){
        vector<uint32_t> l=Lists[i];
        int index=0;
        for(int j=0;j<Lists[0].size();j++){
            int element=Lists[0][j];
            bool found=binarySearch(l,element);
            if(!found)
             Lists[0].erase(Lists[0].begin()+j--);           

        }
    }
    return Lists[0];
}