#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <pthread.h>
using namespace std;

vector<vector<uint32_t>> Lists;// 倒排列表
vector<vector<uint32_t>> bitmap;// 位图存储的倒排列表

//****************************************静态线程_位图算法**********************************************
const int pthread_Bitmap_Static_thread_count = 8;
typedef struct threadParam_Bitmap_Static{
   int t_id;
} threadParam_Bitmap_Static;

void* threadFunc_bitmap_S(void* param) {//两条列表（list[0]和list[i]）求交过程中将查找元素分配给不同线程执行
    threadParam_Bitmap_Static* p = static_cast<threadParam_Bitmap_Static*>(param);
    int t_id = p->t_id;
        
    int n =bitmap[0].size()/pthread_Bitmap_Static_thread_count; 
    int start = t_id * n;


    //***********普通位图*************
    for(int i=1;i<bitmap.size();i++){
      for(int j = start; j < start + n; j++)
        bitmap[0][j] &= bitmap[i][j];  
    }

    pthread_exit(NULL);
}

vector<uint32_t> list_wise_Bitmap_pthread_Static(vector<vector<uint32_t>> &bitmap, int thread_count){
   
        pthread_t handles[thread_count];
        threadParam_Bitmap_Static param[thread_count];

        for(int t_id = 0; t_id < thread_count; t_id++){
            param[t_id].t_id = t_id;
            pthread_create(&handles[t_id], NULL, threadFunc_bitmap_S, &param[t_id]);
        }

        for(int t_id = 0;t_id < thread_count;t_id++){
            pthread_join(handles[t_id], NULL);
        }        

    return bitmap[0];
}