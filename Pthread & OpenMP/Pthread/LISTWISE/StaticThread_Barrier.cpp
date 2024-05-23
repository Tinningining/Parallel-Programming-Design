#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <pthread.h>
using namespace std;

vector<vector<uint32_t>> Lists;// 倒排列表

//*************************************静态线程+barrier版本*****************************************
const int pthread_Barrier_thread_count = 2;
//定义barrier
pthread_barrier_t barrier_Intersection;
pthread_barrier_t barrier_Elimination;

typedef struct threadParam_ord_Static{
   int t_id;
} threadParam_ord_Static;

void* threadFunc_ord_barrier(void* param) {
    threadParam_ord_Static* p = static_cast<threadParam_ord_Static*>(param);
    int t_id = p->t_id;
    
    for (int i = 1; i < Lists.size(); i++) {
        //循环划分任务
        int n = Lists[0].size() / pthread_Barrier_thread_count;
        int start = t_id * n;
        for (int j = start; j < start + n; j++) {
          int element = Lists[0][j];
          bool found = std::binary_search(Lists[i].begin(), Lists[i].end(), element);
          if (!found)
            Lists[0][j] = -1;
        }

        pthread_barrier_wait(&barrier_Intersection);

        //0号线程执行元素消除操作
        if(t_id == 0){
          for (int j = 0; j < Lists[0].size(); j++) {
            if(Lists[0][j] == -1)
              Lists[0].erase(Lists[0].begin() + j--);
          }
        }
        
        pthread_barrier_wait(&barrier_Elimination); 
    }

    pthread_exit(NULL);
}

vector<uint32_t> list_wise_pthread_Barrier(vector<vector<uint32_t>> &Lists, int thread_count) {
    //初始化 barrier
    pthread_barrier_init(&barrier_Intersection, NULL, thread_count);
    pthread_barrier_init(&barrier_Elimination, NULL, thread_count);
    //创建线程
    pthread_t handles[thread_count];//创建对应的Handle
    threadParam_ord_Static param[thread_count];//创建对应的线程数据结构
    for (int t_id = 0; t_id < thread_count; t_id++) {
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id], NULL, threadFunc_ord_barrier, &param[t_id]);
    }  

    for(int t_id = 0;t_id < thread_count;t_id++) {
        pthread_join(handles[t_id], NULL);
    }

    //销毁barrier
    pthread_barrier_destroy(&barrier_Intersection);
    pthread_barrier_destroy(&barrier_Elimination);
    return Lists[0];

}