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


vector<int> element_wise_binary(vector<vector<uint32_t>>vec){//按元素求交
    vector<int> result;
    bool found = false;
    for(int i = 0;i < vec[0].size();i++){
        found = false;
        for(int j = 1;j < vec.size();j++){
            found = binarySearch(vec[j],vec[0][i]);
            if(!found) break;
            if(found && j == vec.size() - 1) result.push_back(vec[0][i]);
        }
    }
    return result;
}