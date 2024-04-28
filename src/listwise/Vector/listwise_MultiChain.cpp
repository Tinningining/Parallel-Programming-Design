#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
using namespace std;

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

vector<uint32_t> list_wise_multichain(vector<vector<uint32_t>> &Lists){//按表求交多路链算法   
    for(int i=2;i<Lists.size()/2;i++){
        vector<uint32_t> l0=Lists[i];
        vector<uint32_t> l1=Lists[i+1];
        
        //偶数表求交
        int index0=0;
        for(int j=0;j< Lists[0].size();j++){
            int element= Lists[0][j];
            //bool found=find(l0,element);
            //bool found=find(l0,element,index0);
            bool found=binarySearch(l0,element);
            if(!found)
                 Lists[0].erase(Lists[0].begin()+j--);
        }

        //奇数表求交
        int index1=0;
        for(int j=0;j< Lists[1].size();j++){
            int element=Lists[1][j];
            //bool found=find(l1,element);
            //bool found=find(l1,element,index1);
            bool found=binarySearch(l1,element);
            if(!found)
                Lists[1].erase(Lists[1].begin()+j--);
        }
    }

    for(int j=0;j<Lists[0].size();j++){
            int element=Lists[0][j];
            bool found=binarySearch(Lists[1],element);
            if(!found)
                Lists[0].erase(Lists[0].begin()+j--);
        }

    return Lists[0];
}