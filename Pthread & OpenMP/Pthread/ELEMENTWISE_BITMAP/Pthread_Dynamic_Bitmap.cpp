#include <iostream>
#include <vector>
#include <pthread.h>
using namespace std;

const int NUM_THREADS = 4; //线程数量
int NUM_TASKS; //任务数量
int size; //划分粒度
int taskPointer = 0; //任务指针
pthread_mutex_t mutex_task; //锁

struct ThreadData1 {
    vector<vector<uint32_t>> bitLists;
    vector<uint32_t> result;
    int t_id; 
};

struct ThreadData2 {
    vector<uint32_t> bitListsTranspose;
    vector<int> result;
    int testNumber;
    int t_id; 
};

    //*********************************************平均划分*******************************************

void* transposeBitmapThread(void* arg) {
    ThreadData1* data = (ThreadData1*)arg;
    vector<vector<uint32_t>> bitLists = data->bitLists;
    int t_id = data->t_id;
    
    //平均划分
    int num = bitLists[0].size()*sizeof(unsigned int)*8;
    int step = num / NUM_THREADS;
    int start = t_id * step;
    int end = (t_id == NUM_THREADS - 1) ? num : (t_id + 1) * step;
    
    for (int i = start; i < end; ++i) {
        data->result.push_back(0);
        for (int j = 0; j < bitLists.size(); ++j) {
            bool bitSet = bitLists[j][i / (sizeof(uint32_t) * 8)] & (1 << (sizeof(uint32_t) * 8 - 1 - (31 - i) % (sizeof(uint32_t) * 8)));
            data->result[i - start] = (data->result[i - start] << 1) + bitSet;
        }
    }
    
    pthread_exit(NULL);
}

vector<uint32_t> transpose_bitmap_pthread(vector<vector<uint32_t>> bitLists, int numThreads) {
    vector<uint32_t> bitListsTranspose;
    pthread_t threads[numThreads];
    ThreadData1 threadData[numThreads];
    
    for (int i = 0; i < numThreads; ++i) {
        threadData[i].bitLists = bitLists;
        threadData[i].result = bitListsTranspose;
        threadData[i].t_id = i;
        pthread_create(&threads[i], NULL, transposeBitmapThread, (void*)&threadData[i]);
    }
    
    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
        bitListsTranspose.insert(bitListsTranspose.end(), threadData[i].result.begin(), threadData[i].result.end());
    }
    
    return bitListsTranspose;
}

void* elementWiseBitmapThread(void* arg) {
    ThreadData2* data = (ThreadData2*)arg;
    vector<uint32_t> bitListsTranspose = data->bitListsTranspose;
    int testNumber = data->testNumber;
    int t_id = data->t_id;

    //平均划分
    int step = bitListsTranspose.size() / NUM_THREADS;
    int start = t_id * step;
    int end = (t_id == NUM_THREADS - 1) ? bitListsTranspose.size() : (t_id + 1) * step;
    
    for (int i = start; i < end; ++i) {
        if (bitListsTranspose[i] == testNumber)
            data->result.push_back(i);
    }
    
    pthread_exit(NULL);
}

vector<int> element_wise_bitmap_pthread(vector<uint32_t> bitListsTranspose, int testNumber, int numThreads) {
    vector<int> intersection;
    pthread_t threads[numThreads];
    ThreadData2 threadData[numThreads];
    
    for (int i = 0; i < numThreads; ++i) {
        threadData[i].bitListsTranspose = bitListsTranspose;
        threadData[i].result = intersection;
        threadData[i].testNumber = testNumber;
        threadData[i].t_id = i;
        pthread_create(&threads[i], NULL, elementWiseBitmapThread, (void*)&threadData[i]);
    }
    
    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
        intersection.insert(intersection.end(), threadData[i].result.begin(), threadData[i].result.end());
    }
    
    return intersection;
}

    //*********************************************动态划分*******************************************

void* elementWiseBitmapThread_D(void* arg) {
    ThreadData2* data = (ThreadData2*)arg;
    vector<uint32_t> bitListsTranspose = data->bitListsTranspose;
    int testNumber = data->testNumber;
    
    //动态划分
    size = bitListsTranspose.size() / NUM_TASKS;
    while (true) {
        int start, end;

        // 动态获取一批任务
        pthread_mutex_lock(&mutex_task);
        start = taskPointer;
        taskPointer += size;
        pthread_mutex_unlock(&mutex_task);

        if (start >= bitListsTranspose.size())
            break; // 所有任务已完成
        end = min(start + size, (int)bitListsTranspose.size());

        for (int i = start; i < end; ++i) {
            if (bitListsTranspose[i] == testNumber)
                data->result.push_back(i);
        }
    }
    
    pthread_exit(NULL);
}

vector<int> element_wise_bitmap_pthread_D(vector<uint32_t> bitListsTranspose, int testNumber, int numThreads) {
    vector<int> intersection;
    pthread_t threads[numThreads];
    ThreadData2 threadData[numThreads];
    
    for (int i = 0; i < numThreads; ++i) {
        threadData[i].bitListsTranspose = bitListsTranspose;
        threadData[i].result = intersection;
        threadData[i].testNumber = testNumber;
        pthread_create(&threads[i], NULL, elementWiseBitmapThread_D, (void*)&threadData[i]);
    }
    
    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
        intersection.insert(intersection.end(), threadData[i].result.begin(), threadData[i].result.end());
    }
    
    taskPointer = 0;

    return intersection;
}