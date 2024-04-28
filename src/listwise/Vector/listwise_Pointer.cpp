#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
using namespace std;

//*************************************listwise_pointer*******************************************
bool find(const vector<uint32_t>list,int element,int &pointer){
    while(list[pointer]<element&&pointer<list.size()-1)
        pointer++;
    if(list[pointer]==element)
            return true;
    return false;
}

vector<uint32_t> list_wise(vector<vector<uint32_t>> &Lists){//按表求交
    for(int i=1;i<Lists.size();i++){
        vector<uint32_t> l=Lists[i];
        int pointer=0;
        for(int j=0;j<Lists[0].size();j++){
            int element=Lists[0][j];
            bool found=find(l,element,pointer);
            if(!found)
             Lists[0].erase(Lists[0].begin()+j--);           

        }
    }
    return Lists[0];
}