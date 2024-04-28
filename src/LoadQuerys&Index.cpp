#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include<set>
#include <cstdint>
#include <algorithm>
using namespace std;
int QueryNumber = 1000; // 要处理的Query数量

void loadQuerys(vector<vector<int>>& Querys){// 加载ExpQuery.txt到二维vector数组Querys中
    ifstream inputFile1("/home/data/search_engine/ExpQuery");
    if (!inputFile1.is_open()) 
        cerr << "Failed to open file." << endl;

    for (int i = 1; i <= QueryNumber; i++) {// 逐行读取文件内容
        vector<int> Query;
        string line;
        if (!getline(inputFile1, line)) 
            cerr << "Failed to read line " << QueryNumber << " from file." << endl;

        istringstream iss(line);
        int number;
        while (iss >> number)// 使用字符串流解析当前行的整数 
            Query.push_back(number);

        std::set<int> uniqueQuery(Query.begin(),Query.end());//去重
        Query.assign(uniqueQuery.begin(),uniqueQuery.end());

        std::sort(Query.begin(), Query.end());// 对向量升序排序
        Querys.push_back(Query);
    }
    inputFile1.close(); // 关闭文件
}


void sort(vector<vector<uint32_t>> &Lists){//按照倒排列表的长度对列表进行升序排序
    for(int i = 0;i < Lists.size();i++)
     for(int j = 0;j < Lists.size()-i-1;j++)  
      if(Lists[j].size() > Lists[j+1].size())           
       swap(Lists[j],Lists[j+1]); 
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
    for(int i = 0;i < Query.size();i++){
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

void showresult(vector<int>result){
    for(int i=0;i<result.size();i++)
      cout<<result[i]<<" ";
    cout<<endl;
}

// int main(){
//     vector<vector<int>> Querys;
//     loadQuerys(Querys);
//     return 0;
// }