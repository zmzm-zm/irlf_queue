#include <iostream>
#include <queue>
#include <thread>
#include <array>
#include <mutex>
#include <chrono>
#include "irlf_queue.hpp"

int main() {
    std::cout << "single thread, 50w data\n";
    const int n = 500'000;
    std::queue<int> std_queue;
    orin::irlf_queue<int> irlf_queue;
    std::cout << "std::queue<int>:\n";
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
        std_queue.push(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed.count() << " milliseconds\n";
    std::cout << "irlf_queue:\n";
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n; ++i) {
        irlf_queue.push(i);
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed.count() << " milliseconds\n";

    std::cout << "\nMultithreading, 50w data, 20 threads\n";
    constexpr int thread_num = 20;
    std::array<std::thread, thread_num> std_threads;
    std::array<std::thread, thread_num> irl_threads;
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

    std::cout << "irlf_queue:\n";
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < thread_num; ++i) {
        irl_threads[i] = std::thread([&irlf_queue, i, n, thread_num]() {
            for (int j = 0; j < n / thread_num; ++j) {
                irlf_queue.push(i * 10'000 + j);
            }
        });
    }
    for (int i = 0; i < thread_num; ++i) {
        irl_threads[i].join();
    }
    end = std::chrono::high_resolution_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed.count() << " milliseconds\n";
    return 0;
}