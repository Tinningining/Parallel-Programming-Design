#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <math.h>
#include <omp.h>
using namespace std;

vector<vector<uint32_t>> Lists;// 倒排列表
vector<vector<uint32_t>> bitmap;// 位图存储的倒排列表
vector<vector<uint32_t>> secondIndex;//二级索引
//线程数量
const int thread_count = 8;
//动态任务分配粒度
const int dsize = 50;

//******************************************OpenMP_Dynamic**************************************************
vector<uint32_t> OpenMP_Dynamic(){
    int size = sqrt(bitmap[0].size());

    for(int i=1;i<secondIndex.size();i++){
        #pragma omp parallel for num_threads(thread_count) //schedule(dynamic, dsize) //动态任务分配方式
        for(int j=0;j<secondIndex[0].size();j++){

            if(secondIndex[0][j] && secondIndex[i][j])//该区块有相同元素
            for(int k = 0; k<size; k++) bitmap[0][j*size+k] &= bitmap[i][j*size+k]; 

            else 
            for(int k = 0; k<size; k++) bitmap[0][j*size+k] = 0;
        }
    }
    return bitmap[0];
}

//******************************************OpenMP_Dynamic**************************************************
vector<uint32_t> OpenMP_Static(){
    int size = sqrt(bitmap[0].size());
    #pragma omp parallel num_threads(thread_count) //schedule(dynamic, dsize) //动态任务分配方式
    for(int i=1;i<secondIndex.size();i++){
        #pragma omp for
        for(int j=0;j<secondIndex[0].size();j++){

            if(secondIndex[0][j] && secondIndex[i][j])//该区块有相同元素
            for(int k = 0; k<size; k++) bitmap[0][j*size+k] &= bitmap[i][j*size+k]; 

            else 
            for(int k = 0; k<size; k++) bitmap[0][j*size+k] = 0;
        }
    }
    return bitmap[0];
}