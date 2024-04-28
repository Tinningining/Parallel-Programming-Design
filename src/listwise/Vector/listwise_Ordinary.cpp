#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
using namespace std;

//****************************************listwise_ordinary*******************************************
bool find(const vector<uint32_t>list,int element){
    for(int i=0;i<list.size();i++)
        if(list[i]==element)
            return true;
    return false;
}

vector<uint32_t> list_wise(vector<vector<uint32_t>> &Lists){//按表求交
    for(int i=1;i<Lists.size();i++){
        vector<uint32_t> l=Lists[i];
        int index=0;
        for(int j=0;j<Lists[0].size();j++){
            int element=Lists[0][j];
            bool found=find(l,element);
            if(!found)
             Lists[0].erase(Lists[0].begin()+j--);           

        }
    }
    return Lists[0];
}


// int main(){
//     vector<vector<uint32_t>> Lists;// 倒排列表
//     vector<vector<uint32_t>> bitmap;// 位图存储的倒排列表
//     struct timeval start, end;
//     for(int i=0;i<QueryNumber;i++){//
//         vector<int>Query=Querys[i];//Querys[i]为第i+1条查询，对应ExpQuery.txt的第i+1行
//         loadIndex(Query,Lists);

//         gettimeofday(&start, NULL);

//         vector<uint32_t>result = list_wise(Lists);
             
//         gettimeofday(&end, NULL);
      
//         double interval = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
//         listwise_ordPointerTime+=interval;
        
//         cout<<"ord第"<<i+1<<"条查询的结果为:"<<endl;
//         showresult(result);
//         Lists.clear();
//     }
    
//     cout << "listwise_ordTime:" << listwise_ordTime << "ms" << endl;
//     return 0;
// }

    