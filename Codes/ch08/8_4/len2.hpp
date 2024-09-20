#include <cstddef>

// 原始数组元素数量
template<typename T, unsigned N>
std::size_t len(T(&)[N])
{
    return N;
}

// 对于有 size_type 类型的元素数量
template<typename T>
typename T::size_type len(T const &t)
{
    return t.size();
}

// 任何类型
std::size_t len(...)
{
    return 0;
}