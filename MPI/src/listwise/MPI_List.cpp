#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <mpi.h>
using namespace std;
int QueryNumber = 1000; // 要处理的Query数量
vector<vector<int>> Querys;
vector<vector<uint32_t>> Lists;// 倒排列表

int world_size, world_rank;
vector<uint32_t> list_wise(){//按表求交
    int n =Lists[0].size()/world_size;//每个线程承担的任务规模
    int start = world_rank * n;
    vector<uint32_t> local_data(Lists[0].begin()+start,Lists[0].begin()+start+n); //分配元素
    
    for(int i=1;i<Lists.size();i++){
        for(int j = 0; j < n; j++){
            int element=local_data[j];
            bool found = std::binary_search(Lists[i].begin(), Lists[i].end(), element);
            if(!found) 
                local_data[j]=-1;
        }
    }
    return local_data;
}


int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int numbers[]={10};
    
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    loadQuerys(Querys);//所有进程均加载Querys
    for(int i=0;i<sizeof(numbers)/sizeof(int);i++)
    {
        QueryNumber=numbers[i];

        if(world_rank == 0){
            cout<<"QueryNumber="<<QueryNumber<<endl;
        }

        double start_time, end_time, elapsed_time;
        // 记录起始时间
        
        for(int i=0;i<QueryNumber;i++)
        {    
            int rows = 0, cols = 0;

            //0号进程加载位图
            if(world_rank == 0){   
                vector<int>Query=Querys[i];   
                loadIndex(Query,Lists);
                rows = Lists.size();
                cols = Lists[0].size();
            }

            //0号进程将位图数据广播到其他进程
            MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
            
            //其他进程根据广播的行数和列数分配内存
            if (world_rank != 0) {
                Lists.resize(rows, vector<uint32_t>(cols));
            }

             //广播 Lists 的数据
            for (int i = 0; i < rows; ++i) {
                MPI_Bcast(Lists[i].data(), cols, MPI_UINT32_T, 0, MPI_COMM_WORLD);
            }
            
            // 记录开始时间
            start_time = MPI_Wtime();

            // 进程内求交
            vector<uint32_t> local_data = list_wise();

            std::vector<uint32_t> all_data(world_size * local_data.size());

            // //*************************非阻塞通信****************************
            // MPI_Request request;
            // MPI_Status status;
            // MPI_Igather(local_data.data(), local_data.size(), MPI_UINT32_T, all_data.data(), local_data.size(), MPI_UINT32_T, 0, MPI_COMM_WORLD, &request);
            // MPI_Wait(&request, &status);//等待非阻塞通信操作完成

            
            // //*****************************阻塞通信*****************************
            // MPI_Gather(local_data.data(), local_data.size(), MPI_UINT32_T, all_data.data(), local_data.size(), MPI_UINT32_T, 0, MPI_COMM_WORLD);    
            
            

            //**************************单边通信****************************
            int data_size = local_data.size();
            MPI_Win win;
            MPI_Win_create(all_data.data(), data_size * world_size * sizeof(uint32_t), sizeof(uint32_t), MPI_INFO_NULL, MPI_COMM_WORLD, &win);

            MPI_Win_fence(0, win); // Start a new epoch

            // Put local_data into all_data on rank 0
            MPI_Put(local_data.data(), data_size, MPI_UINT32_T, 0, world_rank * data_size, data_size, MPI_UINT32_T, win);

            MPI_Win_fence(0, win); // End the epoch and ensure all puts have completed

            if (world_rank == 0) {
                // Rank 0 gathers data from all processes
                for (int i = 0; i < world_size; ++i) {
                    for (int j = 0; j < data_size; ++j) {
                        all_data[i * data_size + j] = all_data[i * data_size + j];
                    }
                }
            }
            MPI_Win_free(&win);

            // 在0号进程中拼接所有数据
            if (world_rank == 0) {
                std::vector<uint32_t> final_result;
                for (int i = 0; i < world_size; i++) {
                    for (int j = 0; j < local_data.size(); j++) {
                        if (all_data[i * local_data.size() + j] != -1) {
                            final_result.push_back(all_data[i * local_data.size() + j]);
                        }
                    }
                }

                // // 打印全局结果（仅在第 0 号进程上）
                // cout<<"第"<<i+1<<"条查询的结果为:"<<endl;
                // showresult(final_result);

                // 记录结束时间
                end_time = MPI_Wtime();

                // 计算执行时间
                elapsed_time += end_time - start_time;
            }

            Lists.clear();
        }

        if(world_rank == 0) {
            printf("Process took %.3f ms\n", elapsed_time*1000);
        }

    }
    MPI_Finalize();
    return 0;
}
