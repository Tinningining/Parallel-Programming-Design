#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <math.h>
#include <pthread.h>
using namespace std;

vector<vector<uint32_t>> Lists;// 倒排列表
vector<vector<uint32_t>> bitmap;// 位图存储的倒排列表
vector<vector<uint32_t>> secondIndex;// 二级索引

//******************************************二级索引普通算法********************************************
void loadSecondIndex(vector<vector<uint32_t>>bitmap, vector<vector<uint32_t>>&secondIndex){
    int size = floor(sqrt(bitmap[0].size()));//向下取整
    for(int i = 0; i<bitmap.size(); i++){
        vector<uint32_t> l(size,0);
        for(int j = 0; j < bitmap[0].size()/size - 1; j++){
            bool allZero = true;

            for(int k = 0; k<size; k++){
                if(bitmap[i][j*size+k] == 1){
                    allZero = false;
                    break;
                }
            }

            if(allZero)
                l[j] = 0;
            else
                l[j] = 1;

        }
        secondIndex.push_back(l);
    }
}


vector<uint32_t> list_wise_secondIndex(vector<vector<uint32_t>>& bitmap, vector<vector<uint32_t>> secondIndex){
    int size = sqrt(bitmap[0].size());

    for(int i=1;i<secondIndex.size();i++){

        for(int j=0;j<secondIndex[0].size();j++){

            if(secondIndex[0][j] && secondIndex[i][j])//该区块有相同元素
            for(int k = 0; k<size; k++) bitmap[0][j*size+k] &= bitmap[i][j*size+k]; 

            else 
            for(int k = 0; k<size; k++) bitmap[0][j*size+k] = 0;
        }
    }
    return bitmap[0];
}



//*******************************************二级索引+Pthread**********************************************
typedef struct threadParam{
   vector<uint32_t> *otherBitmapList; //待求交列表
} threadParam;

int next_arr = 0;
pthread_mutex_t mutex_task;

void* threadFunc(void* param) {
    threadParam* p = static_cast<threadParam*>(param);
    const vector<uint32_t>& otherBitmapList = *(p->otherBitmapList);
    const int size = sqrt(bitmap[0].size());
    //*****动态划分方式*******
    int j = 0; //待执行的任务区块
    while(true){
        pthread_mutex_lock(&mutex_task);
        for(; next_arr < secondIndex[0].size(); next_arr++){
            if(secondIndex[0][next_arr]){
                j = next_arr++;
                break;// 找到了一个要求交的区块
            }
        }
        pthread_mutex_unlock(&mutex_task);

        if(j == secondIndex[0].size()) break;

        for(int k = 0; k<size; k++)
        bitmap[0][j*size+k] &= otherBitmapList[j*size+k]; //按位与
    }
}

vector<uint32_t> pthread(vector<vector<uint32_t>> &bitmap, vector<vector<uint32_t>> secondIndex, int thread_count){
    pthread_mutex_init(&mutex_task, NULL);
    for(int i=1;i<secondIndex.size();i++) {
        for(int j=0;j<secondIndex[0].size();j++)
            secondIndex[0][j] &= secondIndex[i][j]; //二级索引按位与

        pthread_t handles[thread_count];
        threadParam param[thread_count];  

        for(int t_id = 0;t_id < thread_count;t_id++){
            param[t_id].otherBitmapList = &bitmap[i];
            pthread_create(&handles[t_id], NULL, threadFunc, &param[t_id]);
        }

        for(int t_id = 0;t_id < thread_count;t_id++){
            pthread_join(handles[t_id], NULL);
        }
    }

    return bitmap[0];
}