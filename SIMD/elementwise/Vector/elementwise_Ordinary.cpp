#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>

using namespace std;

bool find(vector<uint32_t>list,int element){
    for(int i=0;i<list.size();i++)
        if(list[i]==element)
            return true;
    return false;
}


vector<int> element_wise(vector<vector<uint32_t>>vec){//按元素求交
    vector<int> result;
    // vector<int> point(vec.size());
    bool found = false;
    for(int i = 0;i < vec[0].size();i++){
        found = false;
        for(int j = 1;j < vec.size();j++){
            found = find(vec[j],vec[0][i]);
            if(!found) break;
            if(found && j == vec.size() - 1) result.push_back(vec[0][i]);
        }
    }
    return result;
}