#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>

using namespace std;

//*********************************listwise_Bitmap_AVX-512*****************************************
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


//***************************************listwise_Bitmap*******************************************
vector<uint32_t> list_wise_bitmap(vector<vector<uint32_t>>bitmap){
    for(int i=1;i<bitmap.size();i++)
      for(int j=0;j<bitmap[0].size();j++)
        bitmap[0][j]&=bitmap[i][j];

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


// int main(){
//     vector<vector<uint32_t>> Lists;// 倒排列表
//     vector<vector<uint32_t>> bitmap;// 位图存储的倒排列表
//     struct timeval start, end;
//     for(int i=0;i<QueryNumber;i++){
//         vector<int>Query=Querys[i];//Querys[i]为第i+1条查询，对应ExpQuery.txt的第i+1行
//         loadIndex(Query,Lists);
//         int size =bitmap_size(Lists)/32+1;
        
//         gettimeofday(&start, NULL);

//         loadBitMap(Lists,bitmap,size);
//         vector<uint32_t> intersection = list_wise_bitmap(bitmap);
//         vector<uint32_t>result=bitmap_to_int(intersection);

//         gettimeofday(&end, NULL);
    
//         double interval = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
//         listwise_BitMapTime+=interval;
   
//         cout<<"bitmap第"<<i+1<<"条查询的结果为:"<<endl;
//         showresult(result);
//         Lists.clear();
//         bitmap.clear();
//     }
//     cout << "listwise_BitMapTime:" << listwise_BitMapTime << "ms" << endl;
//     return 0;
// }