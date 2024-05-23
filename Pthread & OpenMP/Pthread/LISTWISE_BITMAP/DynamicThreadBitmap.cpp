#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <pthread.h>
using namespace std;

vector<vector<uint32_t>> Lists;// 倒排列表
vector<vector<uint32_t>> bitmap;// 位图存储的倒排列表

//****************************************动态线程_位图算法**********************************************
const int pthread_Bitmap_Dynamic_thread_count = 8;
typedef struct threadParam_Bitmap_Dynamic{
   int t_id;
   vector<uint32_t> *otherBitmapList;
} threadParam_Bitmap_Dynamic;

void* threadFunc_bitmap_D(void* param) {//两条列表（list[0]和list[i]）求交过程中将查找元素分配给不同线程执行
    threadParam_Bitmap_Dynamic* p = static_cast<threadParam_Bitmap_Dynamic*>(param);
    int t_id = p->t_id;
    const vector<uint32_t>& otherBitmapList = *(p->otherBitmapList);
    
    int n =bitmap[0].size()/pthread_Bitmap_Dynamic_thread_count; 
    int start = t_id * n;
    
    //***********普通位图*************
    for(int j = start; j < start + n; j++){
      bitmap[0][j] &= otherBitmapList[j];
    }
    
    pthread_exit(NULL);
}

vector<uint32_t> list_wise_Bitmap_pthread_Dynamic(vector<vector<uint32_t>> &bitmap, int thread_count){
    for(int i=1;i<bitmap.size();i++){
        pthread_t handles[thread_count];
        threadParam_Bitmap_Dynamic param[thread_count];

        for(int t_id = 0; t_id < thread_count; t_id++){
            param[t_id].t_id = t_id;
            param[t_id].otherBitmapList = &bitmap[i];
            pthread_create(&handles[t_id], NULL, threadFunc_bitmap_D, &param[t_id]);
        }
            
        for(int t_id = 0;t_id < thread_count;t_id++){
            pthread_join(handles[t_id], NULL);
        }        

    }

    return bitmap[0];
}