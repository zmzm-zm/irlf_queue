# irlf queue
## basic information
this is a single-forward queue  
its structure has a picture in ~/pic/  
and it is lock-free  
ATTENTION, it doesn't have GC  
## member function
- push(T data)
> add a node
- pop()
> pop up a node
> type is **node\***
- size()
> return queue's size
> type is **std::size_t\***  
there are detailed explanation in source code file: `./src/irlf_queue.hpp`