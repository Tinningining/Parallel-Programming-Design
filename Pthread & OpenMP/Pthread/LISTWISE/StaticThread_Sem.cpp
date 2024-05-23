#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

vector<vector<uint32_t>> Lists;// 倒排列表

//*************************************静态线程+信号量版本*****************************************
//静态线程数量
const int pthread_Static_thread_count = 2;
//定义信号量
sem_t sem_main;
sem_t* sem_workerstart = (sem_t*)malloc(pthread_Static_thread_count*sizeof(sem_t));
sem_t* sem_workerend = (sem_t*)malloc(pthread_Static_thread_count*sizeof(sem_t));


typedef struct threadParam_ord_Static{
   int t_id;
} threadParam_ord_Static;


void* threadFunc_ord_S(void* param) {
    threadParam_ord_Static* p = static_cast<threadParam_ord_Static*>(param);
    int t_id = p->t_id;

    for (int i = 1; i < Lists.size(); i++) {
        sem_wait(&sem_workerstart[t_id]);

        //循环划分任务
        int n = Lists[0].size() / pthread_Static_thread_count;
        int start = t_id * n;
        for (int j = start; j < start + n; j++) {
          int element = Lists[0][j];
          bool found = std::binary_search(Lists[i].begin(), Lists[i].end(), element);
          if (!found)
            Lists[0][j] = -1;
        }
        sem_post(&sem_workerend[t_id]);
        sem_wait(&sem_workerstart[t_id]); //阻塞，等待主线程唤醒进入下一轮
    }

    pthread_exit(NULL);
}


vector<uint32_t> list_wise_pthread_Static(vector<vector<uint32_t>> &Lists, int thread_count) {
    //初始化信号量
    sem_init(&sem_main, 0, 0); 
    for (int t_id = 0; t_id < thread_count; t_id++) {
      sem_init(&sem_workerstart[t_id], 0, 0);
      sem_init(&sem_workerend[t_id], 0, 0);
    }

    //创建线程
    pthread_t handles[thread_count];//创建对应的Handle
    threadParam_ord_Static param[thread_count];//创建对应的线程数据结构
    for (int t_id = 0; t_id < thread_count; t_id++) {
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id], NULL, threadFunc_ord_S, &param[t_id]);
    }  

    for (int i = 1; i < Lists.size(); i++) {
        //开始唤醒工作线程
        for (int t_id = 0; t_id < thread_count; t_id++){
          sem_post(&sem_workerstart[t_id]);
        }

        //主线程睡眠（等待所有的工作线程完成此轮求交任务）
        for (int t_id = 0; t_id < thread_count; t_id++){
          sem_wait(&sem_workerend[t_id]);
        }
        
        //主线程消除元素 
        for (int j = 0; j < Lists[0].size(); j++) {
          if(Lists[0][j] == -1)
            Lists[0].erase(Lists[0].begin() + j--);
        }

        //主线程再次唤醒工作线程进入下一轮次的消去任务
        for(int t_id = 0;t_id < thread_count;t_id++) {
          sem_post(&sem_workerstart[t_id]);
        }
        
    }

    for(int t_id = 0;t_id < thread_count;t_id++) {
        pthread_join(handles[t_id], NULL);
    }

    return Lists[0];
}