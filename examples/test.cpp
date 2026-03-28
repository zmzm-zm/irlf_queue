#include "irlf_queue.hpp"
#include <iostream>
#include <thread>
#include <mutex>

int main() {
	constexpr int size = 20;
	orin::irlf_queue<int> q;
	std::thread w_ts[size];
	std::thread r_ts[size];
	std::mutex w_io_mutex;
	std::mutex r_io_mutex;
	for(int i = 0; i < size; ++i) {
		w_ts[i] = std::thread([&q, i, &w_io_mutex](){
			q.push(i);
			std::lock_guard<std::mutex> io_lock(w_io_mutex);
			printf("W_Thread %d writes: %d\n", i, i);
		});
	}
	for(int i = 0; i < size; ++i) {
		r_ts[i] = std::thread([&q, i, &r_io_mutex]() {
			auto r = q.pop();
			std::lock_guard<std::mutex> io_lock(r_io_mutex);
			if (r != nullptr) printf("R_Thread %d reads: %d\n", i, r->_data);
		});
	}
	for(auto& t: r_ts) {
		t.join();
	}
	for(auto& t: w_ts) {
		t.join();
	}

	return 0;
}