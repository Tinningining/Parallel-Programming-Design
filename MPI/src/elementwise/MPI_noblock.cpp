#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <set>
#include <cstdint>
#include <algorithm>
#include <cmath>

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>

#include <mpi.h>

using namespace std;
int QueryNumber; // 要处理的Query数量

void sort(vector<vector<uint32_t>> &Lists){//按照倒排列表的长度对列表进行升序排序
    for(size_t i = 0;i < Lists.size();i++)
     for(size_t j = 0;j < Lists.size()-i-1;j++)  
      if(Lists[j].size() > Lists[j+1].size())           
       swap(Lists[j],Lists[j+1]); 
}

vector<int> loadQuery(int line_number){// 加载ExpQuery.txt到二维vector数组Querys中
    vector<int> Query;
    ifstream inputFile1("/home/data/search_engine/ExpQuery");
    if (!inputFile1.is_open()) 
        cerr << "Failed to open file." << endl;

    string line;

        // // 逐行读取文件内容
    for (int i = 1; i <= line_number; ++i) {
        if (!getline(inputFile1, line))
            cerr << "Failed to read line " << line_number << " from file." << endl;
    }

    // 使用字符串流解析当前行的整数
    istringstream iss(line);
    int number;
    while (iss >> number) {
        Query.push_back(number);
    }
    inputFile1.close(); // 关闭文件

    set<int> uniqueQuery(Query.begin(),Query.end());//去重
    Query.assign(uniqueQuery.begin(),uniqueQuery.end());

    sort(Query.begin(), Query.end());

    return Query;
}

void loadIndex(vector<int> Query,vector<vector<uint32_t>> &Lists){// 根据单条Query读取ExpIndex文件，并将结果加载到Lists中
    ifstream inputFile2("/home/data/search_engine/ExpIndex", ios::binary); // 打开二进制文件
    if (!inputFile2.is_open()) {
        cerr << "Failed to open file." << endl;
    }
    int length;
    int position = 0; // 从文件中提取第1个无符号整数（小端），索引从0开始
    int count = 0;
    int n = 0;
    for(size_t i = 0;i < Query.size();i++){
        vector<uint32_t> list;
        while(count != Query[i]){
            // 计算要移动的字节数
            streampos offset = position * sizeof(uint32_t);

            // 将文件指针移动到相应位置
            inputFile2.seekg(offset);

            if (inputFile2.read(reinterpret_cast<char*>(&length), sizeof(uint32_t))) {
                // 读取文件中的第一个无符号整数（小端）
                position += (length + 1);
            } else {
                cerr << "Failed to read number from file." << endl;
            }
            count++;
        }

        streampos offset = position * sizeof(uint32_t);
        inputFile2.seekg(offset);
        if (inputFile2.read(reinterpret_cast<char*>(&length), sizeof(uint32_t))) {
            // 读取文件中的第一个无符号整数（小端）
            position += (length + 1);
        } else {
            cerr << "Failed to read number from file." << endl;
        }
        count++;

        for(int j = 0;j < length;j ++)
        {
            if (inputFile2.read(reinterpret_cast<char*>(&n), sizeof(uint32_t))) {
                // 读取文件中的第一个无符号整数（小端）
                list.push_back(n);
            } else {
                cerr << "Failed to read number from file." << endl;
            }
        }
        Lists.push_back(list);
    }
    inputFile2.close(); // 关闭文件
    sort(Lists);
}

//*********************************************平凡算法*******************************************

// 二分查找函数
bool binarySearch(const vector<uint32_t> list, uint32_t element) {
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

// 并行求交函数
vector<int> MIP_element_wise_binary(const vector<vector<uint32_t>>& vec, int world_rank, int world_size) {
    vector<int> local_result;
    size_t chunk_size = vec[0].size() / world_size;
    size_t start_idx = world_rank * chunk_size;
    size_t end_idx = (world_rank == world_size - 1) ? vec[0].size() : start_idx + chunk_size;
    bool found;
    // cout << start_idx << " " << end_idx << endl;
    for (size_t i = start_idx; i < end_idx; ++i) {
        found = false;
        for(size_t j = 1;j < vec.size();j++){
            found = binarySearch(vec[j],vec[0][i]);
            if(!found) break;
            if(found && j == vec.size() - 1) local_result.push_back(vec[0][i]);
        }
    }
    return local_result;
}

//*********************************************位图算法*******************************************

const int bitmap_size(vector<vector<uint32_t>>list){//位图列表长度
    vector<int> v;
    for(size_t i=0;i<list.size();i++)
     v.push_back(list[i].back());
    std::sort(v.begin(),v.end());
    return v.back();
}

void loadBitLists(vector<vector<uint32_t>> &Lists,vector<vector<uint32_t>> &bitLists,int size){//加载位图列表
    for(vector<uint32_t> arr : Lists){
        vector<uint32_t> bitmap(size,0);
        for(int num : arr){
            unsigned int bitIndex = (num)/(sizeof(uint32_t)*8);
            int offset = num%(sizeof(uint32_t)*8);
            bitmap[bitIndex] |= (1 << offset);
        }
        bitLists.push_back(bitmap);
    }
}

vector<int> MIP_element_wise_bitmap(const vector<vector<uint32_t>>& bitLists, int test_number, int world_rank, int world_size) {
    int size = bitLists[0].size()*sizeof(unsigned int)*8;
    vector<int> local_result;
    size_t chunk_size = size / world_size;
    size_t remainder = chunk_size % 32;
    if (remainder != 0)
        chunk_size += (32 - remainder);
    size_t start_idx = world_rank * chunk_size;
    size_t end_idx = (world_rank == world_size - 1) ? size : start_idx + chunk_size;
    // cout << start_idx << " " << end_idx << endl;
    int transpose = 0;
    for (size_t i = start_idx; i < end_idx; ++i) {
        transpose = 0;
        for(size_t j = 0;j < bitLists.size();j ++){
            bool bitSet = bitLists[j][i/(sizeof(unsigned int)*8)] & (1 << (sizeof(unsigned int)*8 - 1 - (31-i)%(sizeof(unsigned int)*8)));
            transpose = (transpose << 1) + bitSet;
        }
        if(transpose == test_number)
            local_result.push_back(i);
    }
    return local_result;
}

void showresult(vector<int>result){
    for(size_t i=0;i<result.size();i++)
      cout<<result[i]<<" ";
    cout<<endl;
}

int main(int argc, char* argv[]) {

    vector<vector<uint32_t>> Lists;// 倒排列表
    vector<vector<uint32_t>> bitLists;// 位图存储的倒排列表

    vector<vector<int>> Querys;
    vector<int> numbers = {10,20,30,40,50,60,70,80,90,100};

    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    for(int j = 0;j < 10;j ++){
        QueryNumber = numbers[j];

        if(world_rank == 0)
            cout << "QueryNumber=" << QueryNumber << ":" << endl;

        for(int i = 1;i <= QueryNumber;i++)
            Querys.push_back(loadQuery(i));

        struct timeval start, end;
        double element_bitmap_Time=0.0;
        double element_ord_binary_Time=0.0;

        // ******************平凡算法*********************

        for(size_t i=0;i<Querys.size();i++){//

            // MPI_Barrier(MPI_COMM_WORLD);

            vector<int>Query=Querys[i];//Querys[i]为第i+1条查询，对应ExpQuery.txt的第i+1行

            // MPI_Barrier(MPI_COMM_WORLD);

            // 所有进程都加载倒排列表
            if (world_rank == 0)
                loadIndex(Query, Lists);

            // 广播 Lists 数据
            size_t Lists_size = Lists.size();
            MPI_Bcast(&Lists_size, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
            if (world_rank != 0) Lists.resize(Lists_size);
            for (size_t i = 0; i < Lists_size; ++i) {
                size_t inner_size = Lists[i].size();
                MPI_Bcast(&inner_size, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
                if (world_rank != 0) Lists[i].resize(inner_size);
                MPI_Bcast(Lists[i].data(), inner_size, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
            }


            // MPI_Barrier(MPI_COMM_WORLD);
    
            // 获取开始时间
            if (world_rank == 0) 
                gettimeofday(&start, NULL);

            // MPI_Barrier(MPI_COMM_WORLD);

            vector<int> local_result = MIP_element_wise_binary(Lists, world_rank, world_size);

            // MPI_Barrier(MPI_COMM_WORLD);

            // 使用MPI_Igather和MPI_Igatherv收集结果
            int local_size = local_result.size();
            vector<int> recvcounts(world_size);
            MPI_Request req_size, req_gatherv;
            MPI_Igather(&local_size, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD, &req_size);
            
            vector<int> displs(world_size, 0);
            if (world_rank == 0) {
                MPI_Wait(&req_size, MPI_STATUS_IGNORE);
                int total_size = 0;
                for (int i = 0; i < world_size; ++i) {
                    displs[i] = total_size;
                    total_size += recvcounts[i];
                }
            }

            vector<int> global_result;
            if (world_rank == 0) 
                global_result.resize(displs[world_size - 1] + recvcounts[world_size - 1]);

            MPI_Igatherv(local_result.data(), local_size, MPI_INT, global_result.data(), recvcounts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD, &req_gatherv);

            // 等待非阻塞收集完成
            MPI_Wait(&req_gatherv, MPI_STATUS_IGNORE);

            if (world_rank == 0) {
                gettimeofday(&end, NULL);
                double interval = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
                element_ord_binary_Time+=interval;
                // cout<<"binary第"<<i+1<<"条查询的结果为:"<<endl;
                // showresult(global_result);
                Lists.clear();
                bitLists.clear();
            }

            // MPI_Barrier(MPI_COMM_WORLD);

        }

        if (world_rank == 0)
            cout << "binary_time_MPI:" << element_ord_binary_Time << "ms" << endl;

        // MPI_Barrier(MPI_COMM_WORLD);

        //******************位图算法*********************

        for(size_t i=0;i<Querys.size();i++){//

            // MPI_Barrier(MPI_COMM_WORLD);

            vector<int>Query=Querys[i];//Querys[i]为第i+1条查询，对应ExpQuery.txt的第i+1行

            // MPI_Barrier(MPI_COMM_WORLD);

            loadIndex(Query, Lists);

            // MPI_Barrier(MPI_COMM_WORLD);
    
            // 获取开始时间
            if (world_rank == 0) 
                gettimeofday(&start, NULL);

            // MPI_Barrier(MPI_COMM_WORLD);

            int size = bitmap_size(Lists) / 32 + 1;

            // 所有进程都加载倒排列表
            if (world_rank == 0)
                loadBitLists(Lists,bitLists,size);

            // 广播 bitLists 数据
            size_t Lists_size = bitLists.size();
            MPI_Bcast(&Lists_size, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
            if (world_rank != 0) bitLists.resize(Lists_size);
            for (size_t i = 0; i < Lists_size; ++i) {
                size_t inner_size = bitLists[i].size();
                MPI_Bcast(&inner_size, 1, MPI_UNSIGNED_LONG, 0, MPI_COMM_WORLD);
                if (world_rank != 0) bitLists[i].resize(inner_size);
                MPI_Bcast(bitLists[i].data(), inner_size, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
            }

            int test_number = pow(2,bitLists.size()) - 1;

            vector<int> local_result = MIP_element_wise_bitmap(bitLists, test_number, world_rank, world_size);

            // MPI_Barrier(MPI_COMM_WORLD);

            // 使用MPI_Igather和MPI_Igatherv收集结果
            int local_size = local_result.size();
            vector<int> recvcounts(world_size);
            MPI_Request req_size, req_gatherv;
            MPI_Igather(&local_size, 1, MPI_INT, recvcounts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD, &req_size);
            
            vector<int> displs(world_size, 0);
            if (world_rank == 0) {
                MPI_Wait(&req_size, MPI_STATUS_IGNORE);
                int total_size = 0;
                for (int i = 0; i < world_size; ++i) {
                    displs[i] = total_size;
                    total_size += recvcounts[i];
                }
            }

            vector<int> global_result;
            if (world_rank == 0) 
                global_result.resize(displs[world_size - 1] + recvcounts[world_size - 1]);

            MPI_Igatherv(local_result.data(), local_size, MPI_INT, global_result.data(), recvcounts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD, &req_gatherv);

            // 等待非阻塞收集完成
            MPI_Wait(&req_gatherv, MPI_STATUS_IGNORE);

            if (world_rank == 0) {
                gettimeofday(&end, NULL);
                double interval = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
                element_bitmap_Time+=interval;
                // cout<<"bitmap第"<<i+1<<"条查询的结果为:"<<endl;
                // showresult(global_result);
                Lists.clear();
                bitLists.clear();
            }

            // MPI_Barrier(MPI_COMM_WORLD);

        }
        
        if (world_rank == 0)
            cout << "bitmap_time_MPI:" << element_bitmap_Time << "ms" << endl;


        Querys.clear();
    }

    MPI_Finalize();

    return 0;
}