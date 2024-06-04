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
vector<vector<uint32_t>> bitmap;// 位图存储的倒排列表

//****************************************MPI**********************************************  
int world_size, world_rank;
int thread_count = 4;
vector<uint32_t> list_wise_bitmap(){
    //#pragma omp parallel num_threads(thread_count)
    for(int i=1;i<bitmap.size();i++)
    {
        int n =bitmap[0].size()/world_size;
        int start = world_rank * n;
        //#pragma omp for
        //#pragma omp simd safelen(4) //SIMD
        for(int j = start; j < start + n; j++)
        {
            bitmap[0][j] &= bitmap[i][j];
        }
    }
    return bitmap[0];
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int numbers[]={10,20,30,40,50,60,70,80,90,100};
    
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
        
        for(int i=0;i<QueryNumber;i++)
        {    
            int rows = 0, cols = 0;

            //0号进程加载位图
            if(world_rank == 0){   
                vector<int>Query=Querys[i];   
                loadIndex(Query,Lists);
                int size =bitmap_size(Lists)/32+1;
                loadBitMap(Lists,bitmap,size);
                rows = bitmap.size();
                cols = bitmap[0].size();
                Lists.clear();
            }

            //0号进程将位图数据广播到其他进程
            MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

            //其他进程根据广播的行数和列数分配内存
            if (world_rank != 0) {
                bitmap.resize(rows, vector<uint32_t>(cols));
            }

             //广播 bitmap 的数据
            for (int i = 0; i < rows; ++i) {
                MPI_Bcast(bitmap[i].data(), cols, MPI_UINT32_T, 0, MPI_COMM_WORLD);
            }
            
            // 记录开始时间
            start_time = MPI_Wtime();
            
            vector<uint32_t> temp_result = list_wise_bitmap();
            MPI_Barrier(MPI_COMM_WORLD);


            // 将所有进程的部分结果合并到第 0号进程上
            std::vector<uint32_t> global_result(bitmap[0].size());
            MPI_Reduce(temp_result.data(), global_result.data(), bitmap[0].size(), MPI_UINT32_T, MPI_BAND, 0, MPI_COMM_WORLD);

            // 记录结束时间
            end_time = MPI_Wtime();

            // 计算执行时间
            elapsed_time += end_time - start_time;

            // 打印全局结果（仅在第 0 号进程上）
            if(world_rank == 0) 
            {
                vector<uint32_t>result = bitmap_to_int(global_result);
                // cout<<"Bitmap_Dynamic第"<<i+1<<"条查询的结果为:"<<endl;
                // showresult(result);
            }

            bitmap.clear();
        }

        if(world_rank == 0) {
            printf("Process took %.3f ms\n", elapsed_time*1000);
        }

    }
    MPI_Finalize();
    return 0;
}


