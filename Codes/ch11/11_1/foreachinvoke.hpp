#include <utility>
#include <functional>

template<typename Iter, typename Callable, typename... Args>
void foreach (Iter current, Iter end, Callable op, Args const&... args)
{
    while (current != end) {  // 当还未遍历到元素末尾时
        std::invoke(op,       // 使用传入的可调用体调用
                    args...,  // 任何额外的参数
                    *current); // 以及当前的元素
        ++current;
    }
}