#include <iostream>

// 任意类型两个值的最大值
template <typename T>
T max (T a, T b) 
{
    return b < a ? a : b;
}

// 任意类型三个值的最大值
template <typename T>
T max (T a, T b, T c)
{
    // 会使用模板的 max 函数，哪怕是针对 int 类型
    // 因为后续 int 类型的 max 函数声明太迟
    return max(max(a, b), c);
}

// 两个 int 类型值的最大值
int max (int a, int b)
{
    std::cout << "max(int, int)\n";
    return b < a ? a : b;
}

int main()
{
    ::max(47, 11, 33);      // 会使用 max<T>()，而不是 max(int, int)
}