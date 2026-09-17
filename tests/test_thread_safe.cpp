#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
// #include <chrono>

int gs_your_money = 0; // 金额数
int thread_count = 10; // 线程数
int N = 1000000;       // 每个线程的执行次数

std::mutex glo_mutex;

void add_money(){
    for(int i = 0; i < N; i++){
        std::lock_guard<std::mutex> lock(glo_mutex);
        ++gs_your_money;
    }
}

int main(){
    std::vector<std::thread> threads;

    for(int i = 0; i < thread_count; ++i){
        threads.emplace_back(add_money);
    }

    for(auto& t : threads){
        t.join();
    }

    std::cout << "------------" << gs_your_money << std::endl;

    return 0;
}