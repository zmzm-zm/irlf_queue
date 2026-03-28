#pragma once
#include <cstddef>
#include <atomic>
namespace orin {
/**
 *	@brief protect myself
 *	@details in order to keep my fingers and eyes alive
 *	@{
 */
constexpr auto release = std::memory_order_release;
constexpr auto acquire = std::memory_order_acquire;
constexpr auto relaxed = std::memory_order_relaxed;
/** @} */
template<typename T>
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
	};
	std::atomic<node*> _head;
	std::atomic<node*> _tail;
	node* _head_dummy;
	node* _tail_dummy;
	std::atomic<std::size_t> _size;
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
		node* new_node(new node(data));
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
	}

	/**
	 * @brief pop
	 * @details this version of pop is an old one
	 *			I don't know why it can run properly
	 *			I think it seems that have some problems
	 *			But I test it many times, in Windows and Linux
	 *			even let write thread and read thread run at the same time
	 *			they all runs successfully
	 * @return node which pop out
	 */
	node* pop() {
		if(_size.load(acquire) == 0) return nullptr;
		// avoid read thread consumes reads _head_dummy
		node* tmp = _head_dummy->_next;
		// let _head_dummy points to its next's next
		_head_dummy->_next.store(_head_dummy->_next.load(acquire)->_next, release);
		// size isn't sub now
		// so, size == 1 means no node now
		// then let _tail_dummy points to _head_dummy
		if(_size.load(acquire) == 1) _tail.load(acquire)->_next = _head_dummy;
		_size.fetch_sub(1, release);
		tmp->_next = nullptr;
		return tmp;
	}
	std::size_t size() noexcept {
		return _size.load(acquire);
	}
};}