#include <iostream>
#include <vector>
#include <pthread.h>
using namespace std;

const int NUM_THREADS = 4; //线程数量
int NUM_TASKS; //任务数量
int size; //划分粒度
int taskPointer = 0; //任务指针
pthread_mutex_t mutex_task; //锁

struct ThreadData {
    vector<vector<uint32_t>> vec;
    vector<int> result;
    int t_id; 
};

bool binarySearch(const vector<uint32_t> list, int element) {
    int left = 0;
    int right = list.size() - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (list[mid] == element) {
            return true;
        } else if (list[mid] < element) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return false;
}

    //*********************************************平均划分*******************************************

void* elementWiseThread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    vector<vector<uint32_t>> vec = data->vec;
    int t_id = data->t_id;

    //平均划分
    int step = vec[0].size() / NUM_THREADS;
    int start = t_id * step;
    int end = (t_id == NUM_THREADS - 1) ? vec[0].size() : (t_id + 1) * step;

    bool found;
    for (int i = start; i < end; ++i) {
        found = false;
        for (int j = 1; j < vec.size(); ++j) {
            found = binarySearch(vec[j], data->vec[0][i]);
            if (!found) break;
            if (found && j == vec.size() - 1) data->result.push_back(vec[0][i]);
        }
    }
    
    pthread_exit(NULL);
}

vector<int> element_wise_pthread(vector<vector<uint32_t>> vec, int numThreads) {
    vector<int> result;
    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];
    
    for (int i = 0; i < numThreads; ++i) {
        threadData[i].vec = vec; 
        threadData[i].t_id = i;     
        pthread_create(&threads[i], NULL, elementWiseThread, (void*)&threadData[i]);
    }
    
    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
        result.insert(result.end(), threadData[i].result.begin(), threadData[i].result.end());
    }
    
    return result;
}

    //*********************************************动态划分*******************************************

void* elementWiseThread_D(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    vector<vector<uint32_t>> vec = data->vec;

    //动态划分
    size = vec[0].size() / NUM_TASKS;
    while (true) {
        int start, end;

        // 动态获取一批任务
        pthread_mutex_lock(&mutex_task);
        start = taskPointer;
        taskPointer += size;
        pthread_mutex_unlock(&mutex_task);

        if (start >= vec[0].size())
            break; // 所有任务已完成
        end = min(start + size, (int)vec[0].size());

        for (int i = start; i < end; ++i) {
            bool found = false;
            for (int j = 1; j < vec.size(); ++j) {
                found = binarySearch(vec[j], vec[0][i]);
                if (!found) break;
                if (found && j == vec.size() - 1) data->result.push_back(vec[0][i]);
            }
        }
    }
    pthread_exit(NULL);
}

vector<int> element_wise_pthread_D(const vector<vector<uint32_t>>& vec, int numThreads) {
    vector<int> result;
    pthread_t threads[numThreads];
    ThreadData threadData[numThreads];
    
    // 初始化互斥锁
    pthread_mutex_init(&mutex_task, NULL);
    
    for (int i = 0; i < numThreads; ++i) {
        threadData[i].vec = vec;
        pthread_create(&threads[i], NULL, elementWiseThread_D, (void*)&threadData[i]);
    }
    
    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
        result.insert(result.end(), threadData[i].result.begin(), threadData[i].result.end());
    }
    
    // 销毁互斥锁
    pthread_mutex_destroy(&mutex_task);

    taskPointer = 0;
    
    return result;
}