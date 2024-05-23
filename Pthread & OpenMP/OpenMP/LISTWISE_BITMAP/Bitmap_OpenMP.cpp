#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <omp.h>
using namespace std;

vector<vector<uint32_t>> Lists;// 倒排列表
vector<vector<uint32_t>> bitmap;// 位图存储的倒排列表
//线程数量
const int thread_count = 8;
//动态任务分配粒度
const int dsize = 50;

//****************************************OpenMP动态线程+位图算法**********************************************

std::vector<uint32_t> list_wise_Bitmap_OpenMP_Dynamic() {
    for(int i = 1; i < bitmap.size(); i++) {
        #pragma omp parallel for num_threads(thread_count) //schedule(dynamic, dsize) //动态任务分配方式
        for(int j=0;j<bitmap[0].size();j++)
        bitmap[0][j]&=bitmap[i][j];
    }
    return bitmap[0];
}

//****************************************OpenMP静态线程+位图算法**********************************************
std::vector<uint32_t> list_wise_Bitmap_OpenMP_Static() {
    #pragma omp parallel num_threads(thread_count)
    for(int i = 1; i < bitmap.size(); i++) {
        #pragma omp for //schedule(dynamic, dsize) //动态任务分配方式
        for(int j=0;j<bitmap[0].size();j++)
        bitmap[0][j]&=bitmap[i][j];
    }
    return bitmap[0];
}
