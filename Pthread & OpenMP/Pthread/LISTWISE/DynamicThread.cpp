#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <pthread.h>
using namespace std;

vector<vector<uint32_t>> Lists;// 倒排列表

//***************************************动态线程版本***************************************************
//动态线程数量
const int pthread_Dynamic_thread_count;

typedef struct threadParam_Dynamic{
   int t_id;
   const vector<uint32_t> *otherList;
} threadParam_Dynamic;

void* threadFunc_ord_D(void* param) {//两条列表（list[0]和list[i]）求交过程中将查找元素分配给不同线程执行
    threadParam_Dynamic* p = static_cast<threadParam_Dynamic*>(param);
    int t_id = p->t_id;

    const vector<uint32_t>& otherlist = *(p->otherList);//待查找的列表
    
    int n =Lists[0].size()/pthread_Dynamic_thread_count;//每个线程承担的任务规模

    int start = t_id * n;

    for(int j = start; j < start + n; j++){
        int element=Lists[0][j];
        bool found = std::binary_search(otherlist.begin(), otherlist.end(), element);
        if(!found)
        Lists[0][j] = -1;           
    }

    pthread_exit(NULL);
}

//*****动态划分方式*******
int next_arr = 0;
int size; //划分粒度
pthread_mutex_t mutex_task;

void* threadFunc_ord_D(void* param) {//两条列表（list[0]和list[i]）求交过程中将查找元素分配给不同线程执行
    threadParam_Dynamic* p = static_cast<threadParam_Dynamic*>(param);
    int t_id = p->t_id;
    const vector<uint32_t>& otherlist = *(p->otherList);//待查找的列表
    int task = 0; //待执行的任务

    while(true){
        pthread_mutex_lock(&mutex_task);
        task = next_arr;
        next_arr += size;
        pthread_mutex_unlock(&mutex_task);
        if(task >= Lists[0].size()) break;
        for(int j = task; j < task + size; j++){
            int element = Lists[0][j];
            bool found = std::binary_search(otherlist.begin(), otherlist.end(), element);
            if(!found) Lists[0][j] = -1; 
        }
    }
    pthread_exit(NULL);
}

vector<uint32_t> list_wise_pthread_Dynamic(vector<vector<uint32_t>> &Lists, int thread_count){
    for(int i=1;i<Lists.size();i++){
        pthread_t handles[thread_count];
        threadParam_Dynamic param[thread_count];
        
        for(int t_id = 0;t_id < thread_count;t_id++){
            param[t_id].t_id = t_id;
            param[t_id].otherList = &Lists[i];
            pthread_create(&handles[t_id], NULL, threadFunc_ord_D, &param[t_id]);
        }

        for(int t_id = 0;t_id < thread_count;t_id++){
            pthread_join(handles[t_id], NULL);
        }

        for(int j=0;j<Lists[0].size();j++){
        if(Lists[0][j] == -1)
        Lists[0].erase(Lists[0].begin()+j--);
        }

    }
    return Lists[0];
}