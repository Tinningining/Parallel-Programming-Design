#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <omp.h>
using namespace std;

vector<vector<uint32_t>> Lists;// 倒排列表
//线程数量
const int thread_count = 8;
//动态任务分配粒度
const int dsize = 50;

//*************************************OpenMP静态线程+single版本*****************************************
std::vector<uint32_t> list_wise_OpenMP_Static() {
    #pragma omp parallel num_threads(thread_count)
    {
        for (int i = 1; i < Lists.size(); i++) {

            #pragma omp for //schedule(dynamic, dsize) //动态任务分配方式
            for(int j=0;j<Lists[0].size();j++){
            int element=Lists[0][j];
            bool found=std::binary_search(Lists[i].begin(), Lists[i].end(), element);
            if(!found) Lists[0][j] = -1;
            }

            #pragma omp single
            {
                for (int j = 0; j < Lists[0].size(); j++) 
                if (Lists[0][j] == -1) 
                Lists[0].erase(Lists[0].begin() + j--); //消除元素
            }

        }
    }
    return Lists[0];
}


//*************************************OpenMP静态线程+barrier+master版本*****************************************
vector<uint32_t> list_wise_OpenMP_barrier() {
    #pragma omp parallel num_threads(thread_count)
    {        
        for (int i = 1; i < Lists.size(); i++) {
            
            #pragma omp for //schedule(dynamic, dsize) //动态任务分配方式
            for(int j=0;j<Lists[0].size();j++){
            int element=Lists[0][j];
            bool found=std::binary_search(Lists[i].begin(), Lists[i].end(), element);
            if(!found) Lists[0][j] = -1;
            }

            #pragma omp master
            {
                //消除元素
                for (int j = 0; j < Lists[0].size(); j++) 
                    if (Lists[0][j] == -1) 
                        Lists[0].erase(Lists[0].begin() + j--);       
            }

            #pragma omp barrier
        }
    }

    return Lists[0];
}