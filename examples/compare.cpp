#include <iostream>
#include <queue>
#include <thread>
#include <array>
#include <mutex>
#include <chrono>
#include "lf_queue.hpp"

int main() {
    std::cout << "single thread, 700'000 data\n";
    const int n = 700'000;
    std::queue<int> std_queue;
    ir::lf_queue<int, 128> lf_queue;
    std::cout << "std::queue<int>:\n";
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
        std_queue.push(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed.count() << " milliseconds\n";
    std::cout << "lf_queue:\n";
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
        lf_queue.push(i, ir::policy::SINGLE);
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed.count() << " milliseconds\n";

    std::cout << "\nMultithreading, 700'000 data, 50 threads\n";
    constexpr int thread_num = 50;
    std::array<std::thread, thread_num> std_threads;
    std::array<std::thread, thread_num> lf_threads;
    std::mutex std_mutex;


    std::cout << "std::queue<int>:\n";
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < thread_num; ++i) {
        std_threads[i] = std::thread([&std_mutex, &std_queue, i, n, thread_num]() {
            for (int j = 0; j < n / thread_num; ++j) {
                std::lock_guard<std::mutex> lock(std_mutex);
                std_queue.push(i * 10'000 + j);
            }
        });
    }

    for (int i = 0; i < thread_num; ++i) {
        std_threads[i].join();
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed.count() << " milliseconds\n";

    std::cout << "lf_queue:\n";
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < thread_num; ++i) {
        lf_threads[i] = std::thread([&lf_queue, i, n, thread_num]() {
            for (int j = 0; j < n / thread_num; ++j) {
                lf_queue.push(i * 10'000 + j);
            }
        });
    }
    for (int i = 0; i < thread_num; ++i) {
        lf_threads[i].join();
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed.count() << " milliseconds\n";
    return 0;
}