#pragma once
#include <cstddef>
#include <atomic>
#include "irlf_memory_pool.hpp"
#include "defines.hpp"
namespace orin {
template<typename T, std::size_t memory_pool_size = 10>
class irlf_queue {
	/**
	 *	@brief the structure of my queue
	 *	@details it looks like this:
	 *			|_head_dummy| -> |node1| -> ... -> |nodeN| -> |_tail_dummy|
	 *				^								^			|	  ^
	 *				|								|___________|	  |
	 *			  |_head|										   |_tail|
	 *			both _head and _tail node have a dummy node and points to it
	 *			especially, _tail's dummy node, tail_dummy, points to a node which points to _tail_dummy node
	 *			if you can't understand, look at the text-style picture
	 *			it's just like _tail_dummy points to nodeN and nodeN points to _tail_dummy at the same time
	 *
	 *			ATTENTION, if the picture is messy, you can turn off the 'Automatic line break'(a function of some editors)
	 *			if still, there is a real picture in ~/pic/queue_structure.png
	 */
private:
	struct node {
		node():_next(nullptr){}
		node(T data):_data(data), _next(nullptr){}
		std::atomic<node*> _next;
		T _data;
		static node get() {
			return new node{};
		}
	};
	std::atomic<node*> _head;
	std::atomic<node*> _tail;
	node* _head_dummy;
	node* _tail_dummy;
	std::atomic<std::size_t> _size;
	orin::irlf_memory_pool<node*, memory_pool_size> _pool;
public:
	irlf_queue(): _head(new node()),
				_tail(new node()),
				_head_dummy(new node()),
				_tail_dummy(new node()),
				_size(0){
		/**
		 *	form the basic structure:
		 *			|_head_dummy| ->  |_tail_dummy|
		 *				^	 ^			 |	  ^
		 *				|	 |___________|	  |
		 *			  |_head|				|_tail|
		 *	also, the picture is in ~/pic/basic_queue_structure.png
		 */
		_head.load(relaxed)->_next = _head_dummy;
		_tail.load(relaxed)->_next = _tail_dummy;
		_head_dummy->_next = _tail_dummy;
		_tail_dummy->_next = _head_dummy;
		for (int i = 0; i < memory_pool_size; ++i) {
			_pool.put(new node());
		}
	}
	~irlf_queue() {
		for (int i = 0; i < memory_pool_size; ++i) {
			auto target =  _pool.get();
			if (target != nullptr) delete target;
		}
	}

	/**
	 * @brief push
	 * @details this function is very difficult to understand, emmm... I think it is at least
	 *			so, you should try to know the structure of my queue, it is at the top of this class
	 *
	 *			by the way, if you can understand it by drawing a picture by yourself
	 *			don't listen to me, minds speak louder than words
	 *
	 *			then, I will explain it among the code parts
	 *			please draw a picture as my saying
	 * @param data data you want to push
	 */
	void push(T data) {
		node* new_node = _pool.get();
		bool success = false;
		if (new_node == nullptr) new_node = new node(data);
		else {
			new_node->_data = data;
			success = true;
		}
		// in fact, old == _tail_dummy now
		node* old = _tail.load(acquire)->_next;
		// remember, '_tail->next' == _tail_dummy, so see '_tail.load(acquire)->_next.load(acquire)' as _tail_dummy
		// this line let the last node points to new node
		_tail.load(acquire)->_next.load(acquire)->_next.load(acquire)->_next = new_node;
		// let 'old->_next.load(acquire)' be lvalue(left value), because the first param of compare_exchange_xxx is a left-value reference
		auto old_next = old->_next.load(acquire);
		// compare_exchange_xxx returns false when it failed
		// let _tail_dummy points to new_node
		while (!_tail.load(acquire)->_next.load(acquire)->_next.compare_exchange_weak(old_next, new_node,
																							release, relaxed)) {
			// this will only be run when CAS failed
			// this operation make current last node points to new_node
			_tail.load(acquire)->_next.load(acquire)->_next.load(acquire)->_next = new_node;
		}
		// the same as old_next, let nullptr be lvalue
		node* expected = nullptr;
		// make new_node points to _tail_dummy
		// it runs when this new_node is the latest one
		// if not, it doesn't need to point to _tail_dummy
		// why?
		// because if another write thread add a new node before this CAS, this thread's new node's _next isn't nullptr already
		// and this thread's new node's _next has already been handled properly by another write thread
		// so this CAS only need operate once
		// and use the strong version to avoid 'spurious failures'
		new_node->_next.compare_exchange_strong(expected, _tail.load(acquire)->_next,
												release, relaxed);
		_size.fetch_add(1, release);
		//_pool.put(new_node);
	}

	/**
	 * @brief pop
	 * @details override version
	 * @return node which pop up
	 */
	node* pop() {
		node* old = _head.load(acquire)->_next.load(acquire);
		if (old == _tail.load(acquire)->_next.load(acquire)) return nullptr;
		node* expected = old->_next.load(acquire);
		while(!_head.load(acquire)->_next.load(acquire)->_next.compare_exchange_weak(expected, old->_next.load(acquire)->_next,
																					release, relaxed)) {
			if (old == _tail.load(acquire)->_next.load(acquire)) return nullptr;
		}
		_tail.load(acquire)->_next.load(acquire)->_next.compare_exchange_strong(expected, _head.load(acquire)->_next,
																				release, relaxed);
		node* target = old->_next.load(acquire);
		_pool.put(target);
		return target;
	}
	std::size_t size() noexcept {
		return _size.load(acquire);
	}
};}