#pragma once
#include <cstddef>
#include <atomic>
#include "defines.hpp"
#include "irlf_queue.hpp"
namespace orin {
template<typename T, std::size_t size = 10>
class irlf_memory_pool {
private:
	T _data[size];
	std::atomic<std::size_t> _pos{0};
public:
	irlf_memory_pool() {}
	T get() {
		std::size_t old =  _pos.load(acquire);
		while(old > 0) {
			if(_pos.compare_exchange_weak(old, old - 1, acquire, relaxed)) {
				return _data[old - 1];
			}
		}
		return nullptr;
	}
	void put(T data) {
		std::size_t old =  _pos.load(acquire);
		while(old < size) {
			if(_pos.compare_exchange_weak(old, old + 1, release, relaxed)) {
				_data[old] = data;
				return;
			}
		}
	}
};}