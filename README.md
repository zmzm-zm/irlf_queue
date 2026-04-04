# ir::lf\_queue  
## standard and applicability  
![C++](https://img.shields.io/badge/C++-17-blue)  
![License](https://img.shields.io/badge/license-MIT-green)  
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20Android-lightgrey)  
> [!NOTE]  
> it may still work in **macOS**, **IOS**, **HarmonyOS** and more though untested  

## statement  
this project **is not production-ready**  
it is one of my learning outcomes  
**but you can still use it**  
every version listed here works  
## agreement  
The project doesn't use **fold expressions**  
in this document, `...` is a placeholder and means omitted code, not a fold expression  
and, `lf` means lock-free
## basic information  
### structure  
#### project  
Files:  
- [`lf_queue.hpp`](./src/lf_queue.hpp)  
- [`lf_memory_pool.hpp`](./src/lf_memory_pool.hpp)  
- [`defines.hpp`](./src/defines.hpp)
---
#### classes  
|ir::lf\_queue|public|private|  
|:---:|:---:|:---:|  
|function|`lf_queue()`<br>`~lf_queue()`<br>`push(T data)`<br>`pop()`<br>`size()`|-|  
|variable|-|`node`<br>`_head`<br>`_tail`<br>`_head_dummy`<br>`_tail_dummy`<br>`_size`<br>`_pool`|  
  
|ir::lf\_queue::node|public|private|  
|:---:|:---:|:---:|  
|function|`node()`<br>`node(T data)`<br>`get()`|-|  
|variable|`_next`<br>`_data`|-|  
  
|ir::lf\_memory\_pool|public|private|  
|:---:|:---:|:---:|  
|function|`lf_memory_pool()`<br>`get()`<br>`put(T data)`|-|  
|variable|-|`_data`<br>`_pos`|  
---
### instruction  
#### API  
**ATTENTION**, only user-facing APIs are listed below
|function|param|return|  
|:---:|:---:|:---:|  
|`lf_queue()`|N/A|N/A|  
|`push()`|`T data`| `void`|  
|`pop()`|N/A|`ir::lf_queue::node*`|  
|`size()`|N/A|`std::size_t`|
#### use in action  
```cpp
#include "ir/lf_queue.hpp"
...
ir::lf_queue<int> obj;
// the easiest usage
obj.push(123);
obj.push(456);
// use "SINGLE" to improve the speed
obj.push(321, SINGLE);
// pop() has the same usage
obj.pop();
obj.pop(SINGLE);
// you can retrieve the popped value
int value = obj.pop(SINGLE);
std::cout << value << "\n"; // 321
...
// don't use "SINGLE" in concurrency programming
std::thread ts[20];
for(auto& t: ts) {
	t = std::move(std::thread([&obj](){
		obj.push(1);
	}));
}
... // join them
```
#### attention  
**DO NOT USE "SINGLE" IN CONCURRENCY PROGRAMMING**  
if you use it, `push(T data)` and `pop()` will skip atomic operations  
this will bring many problems
### source code  
code is in these files
- [`lf_queue.hpp`](./src/lf_queue.hpp)  
- [`lf_memory_pool.hpp`](./src/lf_memory_pool.hpp)  
- [`defines.hpp`](./src/defines.hpp)  
the functions `push(T data)` and `pop()` are heavily commented  
the structure is also explained there  
you can find them in [`lf_queue.hpp`](./src/lf_queue.hpp)  
finally, it has an archive named `source code.zip` in the release page of this repository  
## test  
### basic test  
[`test.cpp`](./examples/test.cpp)  
its output:  
![test_output](./pic/test\_output.png)  
### compare with  
#### std::queue  
[`compare.cpp`](./examples/compare.cpp)  
its output:  
![compare_output](./pic/compare\_output.png)  
if you can't view pictures:  
||700'000 data, Single threaded|700'000 data, multithreading|  
|:---:|:---:|:---:|  
|std::queue with std::mutex|4ms|196ms|  
|ir::lf_queue\<int\>|72ms|148ms|  
> [!NOTE]  
> don't use SINGLE param  